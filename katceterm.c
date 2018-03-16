
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "katceterm.h"

#if !defined(HAVE_USLEEP) && defined(HAVE_SELECT)
static void
usleep (unsigned long delay)
{
  struct timeval tv_delay;
  tv_delay.tv_sec = delay / 1000;
  tv_delay.tv_usec = delay - (tv_delay.tv_sec * 1000);
  select (0, (void *) 0, (void *) 0, (void *) 0, &tv_delay);
}
#endif

static char *ser_devices[] =
{
  "/dev/term/a",	/* SunOS >= 5.x */
  "/dev/term/b",
  "/dev/ttya",		/* SunOS <= 4.x */
  "/dev/ttyb",
  "/dev/ttyS0",		/* Linux */
  "/dev/ttyS1",
					/* <- Insert your device here */
  NULL				/* end of array marker */
};

typedef struct pre_convert_speed
{
  int		num_speed;
  speed_t	set_speed;
} convert_speed;

/* this only supports the most common speed settings up to maximum for KatCe */
static convert_speed lookup_speed[] =
{
#ifdef B1200
  {1200, (speed_t) B1200},
#endif
#ifdef B2400
  {2400, (speed_t) B2400},
#endif
#ifdef B4800
  {4800, (speed_t) B4800},
#endif
#ifdef B7200
  {7200, (speed_t) B7200},
#endif
#ifdef B9600
  {9600, (speed_t) B9600},
#endif
#ifdef B19200
  {19200, (speed_t) B19200},
#endif
#ifdef B38400
  {38400, (speed_t) B38400},
#endif
  {0, (speed_t) B0}	/* end of array marker */
};

typedef struct pre_katce_files
{
  /*@null@*//*@dependent@*/ FILE *stream;
  char buffer[512];
  char name[PATH_MAX];
} katce_files;

/* global status */
static katce_files files[7];
static int ser_f = 0;
static int iorslt = 0;
static char serial_line[PATH_MAX];
static speed_t serial_speed = (speed_t) B38400;
static int screen_tab = 8;
static int unix_lines = 1;

#define SLEEPNUM	100

static char
Lies_Katce (void)
{
  char c_in;
  ssize_t ret_val;

  c_in = '\0';

  FTRACE ("Lies_Katce\n");

  ret_val = read (ser_f, &c_in, (size_t) 1);

  if (ret_val < 0)
    {
      (void) fprintf (stderr, gettext("KAT-Ce Send_Katce() error: %s\n"),
		      strerror (errno));
      return ((char) 0);
    }

  RTRACE (c_in);

  return (c_in);

}

static void
Send_Katce (char c_out)
{
  ssize_t ret_val;

  FTRACE ("Send_Katce\n");

  ret_val = write (ser_f, &c_out, (size_t) 1);

  WTRACE (c_out);

  if (ret_val < 0)
    {
      (void) fprintf (stderr, gettext("KAT-Ce Send_Katce() error: %s\n"),
		      strerror (errno));
    }
}

static void
InitCurses (void)
{
  FTRACE ("InitCurses\n");

  (void) initscr ();		/* initialize the curses library */
  (void) nonl ();		/* tell curses not to do NL->CR/NL on output */
  (void) cbreak ();		/* take input chars one at a time, no wait for \n */
  (void) noecho ();		/* don't echo input */
  (void) idlok (stdscr, FALSE);	/* dont allow use of insert/delete line */
}

static void
UninitCurses (void)
{
  FTRACE ("UninitCurses\n");

  (void) endwin ();
}

/* Initialisiert die RS232 Schnittstelle mit 38400 Baud */
/*@maynotreturn@*/
static void
InitSerial (void)
{
  struct termios my_line;

  FTRACE ("InitSerial\n");

  ser_f = open (serial_line, O_RDWR);

  if (ser_f == 0)
    {
      (void) fprintf (stderr,
		      gettext("Could not open serial device (%s), error: %s\n"),
		      serial_line, strerror (errno));
      exit (EXIT_FAILURE);
    }

  (void) tcgetattr (ser_f, &my_line);

  my_line.c_iflag = 0;		/* Transparent input */
  my_line.c_iflag |= (IGNBRK | IGNPAR | IXANY);
  my_line.c_oflag = 0;		/* Transparent output */
  my_line.c_cflag = 0;
/* CRTSXOFF is needed for inbound flow control on Solaris */
#ifndef CRTSXOFF
#define CRTSXOFF 0
#endif
  my_line.c_cflag |= (CRTSCTS | CRTSXOFF | CREAD | CS8);
  my_line.c_lflag = 0;		/* no local modes */

  my_line.c_cc[VMIN] = (cc_t) 1;	/* disable line buffering */

  (void) cfsetospeed (&my_line, serial_speed);
  (void) cfsetispeed (&my_line, serial_speed);

  /*@-shiftimplementation@ */
  (void) tcsetattr (ser_f, (int) TCSADRAIN, &my_line);
  /*@=shiftimplementation@ */
}

static void
UninitSerial (void)
{
  struct termios my_line;

  FTRACE ("UninitSerial\n");

  (void) tcgetattr (ser_f, &my_line);
  (void) cfsetospeed (&my_line, B0);	/* hangup */
  (void) cfsetispeed (&my_line, B0);

  /*@-shiftimplementation@ */
  (void) tcsetattr (ser_f, (int) TCSADRAIN, &my_line);
  /*@=shiftimplementation@ */

  (void) close (ser_f);
}

/* sendet so oft <CR>, bis die KAT-Ce antwortet     */
static void
InitKatce (void)
{
  char c_in_katce;
  ssize_t num_c;
  int c_in_user;
  struct termios my_line;
  cc_t savetime;

  FTRACE ("InitKatce\n");

  (void) addstr (gettext( "Turn on the power of your KatCe now!     <Return>"));

  /* wait for return */
  do
    {
      c_in_user = getch ();
    }
  while (c_in_user != (int) '\r');

  /* set line to not sleep on read */
  (void) tcgetattr (ser_f, &my_line);
  savetime = my_line.c_cc[VTIME];
  my_line.c_cc[VTIME] = (cc_t) 0;
  /*@-shiftimplementation@ */
  (void) tcsetattr (ser_f, (int) TCSADRAIN, &my_line);
  /*@=shiftimplementation@ */

  /* send \r and wait for an answer */
  do
    {
      int count;

      count = 0;
      Send_Katce ('\r');

      do
	    {
	      (void) usleep (100U);
	      num_c = read (ser_f, &c_in_katce, (size_t) 1);
	      count++;
	    }
      while ((count < SLEEPNUM) && (num_c <= 0));
    }
  while (num_c <= 0);

  /* on read do wait for input */
  my_line.c_cc[VTIME] = savetime;
  /*@-shiftimplementation@ */
  (void) tcsetattr (ser_f, (int) TCSADRAIN, &my_line);
  /*@=shiftimplementation@ */
}

/* Sendet die die Bildschirmgroesse an die Kat-Ce */
static void
Setup (void)
{
  char x, y;

  FTRACE ("Setup\n");

  /* get size from curses library and truncate it to katce maxmimum */
  x = (char) (COLS & 0xFF);
  y = (char) ((LINES - 1) & 0xFF);

  Send_Katce (x);
  Send_Katce (y);
}

static void
Drucke (void)
{
  char to_printer;

  FTRACE ("Drucke\n");

  to_printer = Lies_Katce ();
  /* FIXME: not implemented */

  /*
   * 1)   send char to temp file
   * 2)   if char == FormFeed
   * 2.1) close temp file an send it to printer
   * 3)   open new temp file
   *      2.1 and 3 may use Init_Drucker()
   */
}

static void
Init_Drucker (void)
{
  FTRACE ("Init_Drucker\n");

  /* FIXME: not implemented */

  /*
   * 1) check if temp file already open
   * 1.1) if already open close temp file and send to printer
   * 2) open temp file
   */
}

static int
ReadFNr (void)
{
  FTRACE ("ReadFNr\n");

  return ((int) Lies_Katce ());
}

static int
LiesBlockNr (void)
{
  int XH, XL;

  FTRACE ("LiesBlockNr\n");

  XH = (int) Lies_Katce ();
  XL = (int) Lies_Katce ();

  /*@-shiftimplementation@ */
#ifdef WORDS_BIGENDIAN
  return ((XH << 8) | XL);
#else
  return ((XL << 8) | XH);
#endif
  /*@=shiftimplementation@ */
}

static void
ReadFName (char *name)
{
  int name_len;
  int count;

  FTRACE ("ReadFName\n");

  name_len = (int) Lies_Katce ();

  for (count = 0; count < name_len; count++)
    {
      name[count] = Lies_Katce ();
    }
  name[name_len] = (char) 0;
}

static void
LiesBlock (char *buffer)
{
  int count;

  FTRACE ("LiesBlock\n");

  for (count = 0; count < 512; count++)
    {
      buffer[count] = Lies_Katce ();
    }
}

#ifdef NEED_SENDBLOCK		/* unused by now */
/*@unused@*/ static void
SendBlock (char *buffer)
{
  int count;

  FTRACE ("SendBlock\n");

  for (count = 0; count < 512; count++)
    {
      Send_Katce (buffer[count]);
    }
}
#endif

/* Liest den Block [Blocknr] aus dem File [FileNr] und sendet ihn an */
/* die Kat-Ce                                                        */

static void
BlockRead (void)
{
  int FNr;
  long BlNr;
  long Position;
  size_t AnzBytes;

  FTRACE ("BlockRead\n");

  FNr = ReadFNr ();
  if (files[FNr].stream == NULL)
    {
      iorslt = 36;
      return;
    }
  BlNr = 512L * (long) LiesBlockNr ();

  if (BlNr != (Position = ftell (files[FNr].stream)))
    {
      (void) fseek (files[FNr].stream, BlNr, SEEK_SET);	/* FIXME: check return value */
      Position = ftell (files[FNr].stream);
    }

  if (Position >= 0)
    {
      AnzBytes =
	fread (files[FNr].buffer, (size_t) 512U, (size_t) 1U,
	       files[FNr].stream);
      if ((AnzBytes < (size_t) 512U) && (AnzBytes > (size_t) 0U))
	{
	  size_t count;
	  for (count = AnzBytes + 1; count < (size_t) 512U; count++)
	    {
	      files[FNr].buffer[count] = (char) 0;
	    }
	}
      else if (AnzBytes == 0)
	{
	  iorslt = -1;
	}
    }
  else
    {
      iorslt = 36;
    }
}

static void
BlockWrite (void)
{
  int FNr;
  long BlNr;
  long Position;

  FTRACE ("BlockWrite\n");

  FNr = ReadFNr ();
  if (files[FNr].stream == NULL)
    {
      iorslt = 36;
      return;
    }
  BlNr = 512L * (long) LiesBlockNr ();

  LiesBlock (files[FNr].buffer);

  if (BlNr != (Position = ftell (files[FNr].stream)))
    {
      (void) fseek (files[FNr].stream, BlNr, SEEK_SET);	/* FIXME: check return value */
      Position = ftell (files[FNr].stream);
    }

  if (Position >= 0)
    {
      size_t AnzBytes;
      AnzBytes =
	fwrite (files[FNr].buffer, (size_t) 512U, (size_t) 1U,
		files[FNr].stream);
      if (AnzBytes == 0)
	{
	  iorslt = -1;
	}
      else
	{
	  iorslt = 0;
	}
    }
  else
    {
      iorslt = 39;
    }
}

/* OpenFileReadOnly */
static void
ResetFile (void)
{
  int FNr;

  FTRACE ("ResetFile\n");

  FNr = ReadFNr ();

  if (files[FNr].stream != NULL)
    {
      (void) fclose (files[FNr].stream);	/* FIXME: check return value */
      files[FNr].stream = NULL;
    }

  ReadFName (files[FNr].name);

  files[FNr].stream = fopen (files[FNr].name, "r");

  if (files[FNr].stream == NULL)
    {
      iorslt = 30;
    }
}

/* OpenFileWriteOnly */
static void
RewriteFile (void)
{
  int FNr;

  FTRACE ("RewriteFile\n");

  FNr = ReadFNr ();

  if (files[FNr].stream != NULL)
    {
      (void) fclose (files[FNr].stream);	/* FIXME: check return value */
      files[FNr].stream = NULL;
    }

  ReadFName (files[FNr].name);

  files[FNr].stream = fopen (files[FNr].name, "w");

  if (files[FNr].stream == NULL)
    {
      iorslt = 30;
    }
}

/* CloseFile */
static void
CloseFile (void)
{
  int FNr;

  FTRACE ("CloseFile\n");

  FNr = ReadFNr ();

  if (files[FNr].stream != NULL)
    {
      (void) fclose (files[FNr].stream);	/* FIXME: check return value */
      files[FNr].stream = NULL;
    }
}

/* CloseDeleteFile */
static void
Removefile (void)
{
  int FNr;

  FTRACE ("Removefile\n");

  FNr = ReadFNr ();

  if (files[FNr].stream != NULL)
    {
      (void) fclose (files[FNr].stream);	/* FIXME: check return value */
      files[FNr].stream = NULL;
    }

  (void) unlink (files[FNr].name);	/* FIXME: check return value */
}

static void
Send_IOResult (void)
{
  FTRACE ("Send_IOResult\n");

  Send_Katce ((char) iorslt);
  iorslt = 0;
}

static void
Gotoxy (void)
{
  int x, y;

  FTRACE ("Gotoxy\n");

  x = (int) Lies_Katce ();
  y = (int) Lies_Katce ();

  (void) move (y, x);
  (void) refresh ();
}

static void
Sonderfunktion (int *EndofCom)
{
  char c_from_katce;

  FTRACE ("Sonderfunktion\n");

  c_from_katce = Lies_Katce ();

  switch (c_from_katce)
    {
    case 0x00:
      Setup ();
      break;
    case 0x01:
      Drucke ();
      break;
    case 0x02:
      Init_Drucker ();
      break;
    case 0x03:
      BlockRead ();
      break;
    case 0x04:
      BlockWrite ();
      break;
    case 0x05:
      ResetFile ();
      break;
    case 0x06:
      RewriteFile ();
      break;
    case 0x07:
      CloseFile ();
      break;
    case 0x08:
      Removefile ();
      break;
    case 0x09:
      Send_IOResult ();
      break;
    case 0x0a:
      Gotoxy ();
      break;
    case 0x0b:
      *EndofCom = 1;
      break;
    default:
      break;
    }
}

static void
SendKeypr (void)
{
  int c_from_user;

  FTRACE ("SendKeypr\n");

  (void) refresh ();	/* update internal curses buffers */
  timeout (0);			/* do not wait for input */

  c_from_user = getch ();
  if (c_from_user > 0)
    {
      /* at least one char in input queue */
      (void) ungetch (c_from_user);	/* put char back in queue *//* FIXME: check return value */
      timeout (-1);
      Send_Katce ((char) 1);
    }
  else
    {
      /* no char in input queue or other error */
      timeout (-1);
      Send_Katce ((char) 0);
    }
}

static void
SendInput (void)
{
  FTRACE ("SendInput\n");

  (void) refresh ();	/* update internal curses buffers */
  Send_Katce ((char) getch ());
}

static void
ToScreen (char out)
{
  FTRACE ("ToScreen\n");

  switch (out)
    {
    case 0x00:			/* ignore NUL */
      break;
    case 0x0a:			/* NL */
      {
	int x, y;

	getyx (stdscr, y, x);
	(void) move (++y, x);
	break;
      }
    case 0x0c:			/* NP */
      {
	(void) clear ();
	break;
      }
    case 0x0d:			/* CR */
      {
	int x, y;

	getyx (stdscr, y, x);
	(void) move (y, 0);
	break;
      }
    default:
      {
	(void) addch ((chtype) out);
	break;
      }
    }
  (void) refresh ();
}

static speed_t
check_speed_setting (int in_speed)
{
  int count;
  int eo_loop;
  speed_t b_return;

  b_return = B0;
  eo_loop = 0;
  count = 0;
  do
    {
      if (in_speed == lookup_speed[count].num_speed)
	{
	  b_return = lookup_speed[count].set_speed;
	  eo_loop = 1;
	}
      if (lookup_speed[count].num_speed == 0)
	{
	  eo_loop = 1;
	}
      count++;
    }
  while (eo_loop == 0);

  return (b_return);
}

/*@maynotreturn@*/
static int
get_cmd_args (int argc, char /*@notnull@ */ **argv)
{
  int c;
  int ret_val;
  int errflg = 0;
  /*
   * -h                         / print help
   * -t TabWidth                / default 1 tab are 8 spaces
   * -d                         / DOS style file output/input
   * -u [default]               / Unix style file output/input
   * -s SerialLineSpeed         / let user define the default line speed
   * -l SerialLine              / let user define the serial line
   */
  while ((c = getopt (argc, argv, "dhl:s:t:u")) != EOF)
    switch (c)
      {
      case 'd':
	unix_lines = 0;
	(void) fprintf (stderr, gettext( "NOTICE: Option %s is not implemented\n"), "-d");
	break;
      case 'h':
	errflg++;		/* help == error */
	break;
      case 'l':
	if (optarg != NULL)
	  {
	    char x_device[PATH_MAX];
	    ret_val = sscanf (optarg, "%s", x_device);
	    if (ret_val > 0)
	      {
		size_t check_len;
		/* FIXME: check user input */
		/* check_device_setting(x_device); */
		(void) strncpy (serial_line, x_device, (size_t) PATH_MAX - 1);
		serial_line[PATH_MAX - 1] = '\0';
		if (strlen (serial_line) >= (size_t) PATH_MAX - 1)
		  {
		    /* buffer overflow */
		    errflg++;
		  }
	      }
	    else
	      errflg++;
	  }
	else
	  errflg++;
	break;
      case 's':
	if (optarg != NULL)
	  {
	    int x_speed;
	    ret_val = sscanf (optarg, "%d", &x_speed);
	    if (ret_val > 0)
	      {
		serial_speed = check_speed_setting (x_speed);
		if (serial_speed == B0)
		  errflg++;
	      }
	    else
	      errflg++;
	  }
	else
	  errflg++;
	break;
      case 't':
	if (optarg != NULL)
	  {
	    int x_tab;
	    ret_val = sscanf (optarg, "%d", &x_tab);
	    if (ret_val > 0)
	      {
		/* FIXME: check user input */
		/* check_tab_setting(x_tab); */
		screen_tab = x_tab;
		(void) fprintf (stderr,
				gettext( "NOTICE: Option %s is not implemented\n"), "-t");
	      }
	    else
	      errflg++;
	  }
	else
	  errflg++;
	break;
      case 'u':
	unix_lines = 1;
	(void) fprintf (stderr, gettext("NOTICE: Option %s is not implemented\n"), "-u");
	break;
      }

  if (errflg != 0)
    {
      (void) fprintf (stderr,
		      gettext( "usage: %s [-h] [-d] [-u] [-t tab_width] [-l serial_device] [-s serial_speed]\n"),
		      argv[0]);
      exit (EXIT_FAILURE);
    }

  return (EXIT_SUCCESS);
}

int
main (int argc, char **argv)
{
  char c_from_katce;
  int EndofCom = 0;

  FTRACE ("main\n");

  /* initialize global variables */
  serial_line[0] = '\0';

  /* init localisation */
  bindtextdomain(PACKAGE, LOCALEDIR);

  /* get command line options */
  (void) get_cmd_args (argc, argv);

  /* Init screen, serial line, and Kat-Ce communication */
  InitCurses ();
  InitSerial ();
  InitKatce ();

  do
    {
      c_from_katce = Lies_Katce ();
      switch (c_from_katce)
	{
	case 0x11:
	  Sonderfunktion (&EndofCom);
	  break;
	case 0x12:
	  SendKeypr ();
	  break;
	case 0x13:
	  SendInput ();
	  break;
	default:
	  ToScreen (c_from_katce);
	  break;
	}
    }
  while (EndofCom == 0);

  /* reset serial line and curses */
  UninitSerial ();
  UninitCurses ();

  exit (EXIT_SUCCESS);
}

