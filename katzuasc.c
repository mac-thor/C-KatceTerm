
/*
 * convert KatCe files into ASCII files
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "katceterm.h"

static char input_file[PATH_MAX];
static char output_file[PATH_MAX];
static FILE *OutFile;
static FILE *InFile;
static int screen_tab = 8;
static int unix_lines = 1;

static void
convert (void)
{
  char KatBlock[512];
  char ASCZeile[2048];
  int Ende, DLE, Start, Sp;
  size_t i, read_in, ASCLen, write_ret;
  char KatChr;

  ASCLen = 0;
  Ende = 0;
  DLE = 0;
  Start = 1;

  while (feof (InFile) == 0)
    {

      read_in = fread (KatBlock, 1, 512, InFile);

      if (Start == 1)
	{
	  i = 8;		/* ignore file length hint */
	  Start = 0;
	}
      else
	{
	  i = 0;
	}

      while ((Ende != 1) && (i < read_in))
	{
	  KatChr = KatBlock[i];
	  i++;
	  if (DLE == 1)
	    {
	      for (Sp = 0x20; Sp < (int) KatChr; Sp++)
		{
		  ASCZeile[ASCLen++] = ' ';
		}
	      DLE = 0;
	    }			/* end if DLE */
	  else
	    {
	      switch (KatChr)
		{
		case 0x00:	/* end of file marker */
		  {
		    Ende = 1;
		    break;
		  }
		case 0x0d:	/* end of line marker */
		  {
		    ASCZeile[ASCLen++] = (char) 0x0a;
		    ASCZeile[ASCLen] = (char) 0;
		    write_ret = fwrite (&ASCZeile[0], 1, ASCLen, OutFile);
		    if (write_ret != ASCLen)
		      fprintf (stderr, "fwrite %u, %u\n",
			       (unsigned int) write_ret,
			       (unsigned int) ASCLen);
		    ASCLen = 0;
		    break;
		  }
		case 0x10:	/* next char defines number of spaces to insert */
		  {
		    DLE = 1;
		    break;
		  }
		default:		/* normal char */
		  {
		    ASCZeile[ASCLen++] = KatChr;
		    break;
		  }
		}		/* end switch KatChr */
	    }			/* end if !DLE */
	}			/* end while !Ende && i<512 */
    }				/* end while !eof */
}				/* end of convert() function */


/*@maynotreturn@*/
static int
get_cmd_args (int argc, char /*@notnull@ */ **argv)
{
  int c;
  int ret_val;
  int errflg = 0;

  char temp_file[PATH_MAX];
  size_t check_len;

  /*
   * -h				/ print help
   * -t TabWidth			/ default 1 tab are 8 spaces
   * -d				/ DOS style input file
   * -u [default]		/ Unix style input file
   * -i	file			/ Input file (default stdin)
   * -o	file			/ Output File (default stdout)
   */

  while ((c = getopt (argc, argv, "dhi:o:t:u")) != EOF)
    switch (c)
      {
	case 'd':
	  unix_lines = 0;
	  (void) fprintf (stderr, gettext ("NOTICE: Option %s is not implemented\n"), "-d");
	  break;
	case 'h':
	  errflg++;		/* help == error */
	  break;
	case 'i':
	  if (optarg != NULL)
	    {
	      if (strlen (optarg) >= PATH_MAX)	/* frist check length of input */
		errflg++;		/* buffer overflow */
	      else
		{
		  ret_val = sscanf (optarg, "%s", temp_file);
		  if (ret_val > 0)
		    {
		      /* FIXME: check user input */
		      /* check_file(temp_file); */
		      (void) strncpy (input_file, temp_file, (size_t) PATH_MAX - 1);
		      input_file[PATH_MAX - 1] = '\0';
		      if (strlen (input_file) >= (size_t) PATH_MAX - 1)
			errflg++;		/* buffer overflow / name truncated */
		    }
		  else /* string length <= 0 */
		    errflg++;
		} /* end of (string length >= PATH_MAX) */
	    }
	  else /* optarg == NULL */
	    errflg++;
	  break;
	case 'o':
	  if (optarg != NULL)
	    {
	      if (strlen (optarg) >= PATH_MAX)	/* frist check length of input */
		errflg++;		/* buffer overflow */
	      else
		{
		  ret_val = sscanf (optarg, "%s", temp_file);
		  if (ret_val > 0)
		    {
		      /* FIXME: check user input */
		      /* check_file(temp_file); */
		      (void) strncpy (output_file, temp_file, (size_t) PATH_MAX - 1);
		      output_file[PATH_MAX - 1] = '\0';
		      if (strlen (output_file) >= (size_t) PATH_MAX - 1)
			errflg++;		/* buffer overflow / name truncated */
		    }
		  else /* string length <= 0 */
		    errflg++;
		} /* end of (string length >= PATH_MAX) */
	    }
	  else /* optarg == NULL */
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
				  gettext ("NOTICE: Option %s is not implemented\n"), "-t");
		}
	      else /* sscanf faild */
		errflg++;
	    }
	  else /* optarg == NULL */
	    errflg++;
	  break;
	case 'u':
	  unix_lines = 1;
	  (void) fprintf (stderr, gettext ("NOTICE: Option %s is not implemented\n"), "-u");
	  break;
      }	/* end of switch */

  if (errflg != 0)
    {
      (void) fprintf (stderr,
		      gettext ("usage: %s [-h] [-d] [-u] [-t tab_width] [-i inputfile] [-o outputfile]\n"),
		      argv[0]);
      exit (EXIT_FAILURE);
    }

  return (EXIT_SUCCESS);
}

int
main (int argc, char **argv)
{
  input_file[0] = '\0';	/* reset string len to 0 */
  output_file[0] = '\0';	/* reset string len to 0 */

  /* init localisation */
  bindtextdomain(PACKAGE, LOCALEDIR);

  /* get command line options */
  (void) get_cmd_args (argc, argv);

  if (strlen (input_file) == 0)
    InFile = stdin;
  else
    InFile = fopen (input_file, "r");

  if (InFile == NULL)
    {
      fprintf (stderr, gettext ("\nCannot open file %s\n"), input_file);
      exit (EXIT_FAILURE);
    }

  if (strlen (output_file) == 0)
    OutFile = stdout;
  else
    {
      OutFile = fopen (output_file, "w+");
      if (OutFile != NULL)
        (void) fclose (OutFile);
      OutFile = fopen (output_file, "r+");
    }

  if (OutFile == NULL)
    {
      fprintf (stderr, gettext ("\nCannot create file %s\n"), output_file);
      (void) fclose (InFile);
      exit (EXIT_FAILURE);
    }

  convert ();

  (void) fclose (InFile);
  (void) fclose (OutFile);

  exit (EXIT_SUCCESS);
}

