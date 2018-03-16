
/*
 * convert ASCII Files into KatCe files
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "katceterm.h"

static char KatBlock[512];
static char input_file[PATH_MAX];
static char output_file[PATH_MAX];
static FILE *OutFile;
static FILE *InFile;
static size_t BlockP, FileP;
static int screen_tab = 8;
static int unix_lines = 1;

static void
PutKat (char byte)
{
  KatBlock[BlockP] = byte;
  BlockP++;
  FileP++;
  if (BlockP == 512)
    {
      (void) fwrite (KatBlock, 512, 1, OutFile);
      BlockP = 0;
    }
}

static void
convert (void)
{
  char linebuf[1024];

  BlockP = 0;
  FileP = 0;

  /* Startadr */
  PutKat ((char) 0);
  PutKat ((char) 0);
  PutKat ((char) 0);
  PutKat ((char) 0);
  /* Laenge */
  PutKat ((char) 0);
  PutKat ((char) 0);
  PutKat ((char) 0);
  PutKat ((char) 0);

  while (feof (InFile) == 0)
    {
      size_t Zeiger;
      size_t Count;
      size_t LineLen;

      (void) fgets (linebuf, 1024, InFile);
      PutKat ((char) 0x10);	/* Line start marker */
      Zeiger = 0;
      Count = 0;

      /* count number of white spaces at linestart */
      if (strlen (linebuf) > 0)
	{
	  while ((linebuf[Zeiger] == ' ') || (linebuf[Zeiger] == '\t'))
	    {
	      switch (linebuf[Zeiger])
		{
		case ' ':
		  Count++;
		  break;
		case '\t':
		  Count += screen_tab;
		  break;
		}
	      Zeiger++;
	    }
	  if (Count > 0xdf)
	    {
	      Count = 0xdf;	/* overflow: set to max number of spaces */
	    }
	}

  PutKat ((char) (Count + 0x20));	/* write out number of spaces + 0x20 */
  LineLen = strlen (linebuf);
  if (LineLen > 2)
	{
	  while (Zeiger <= (LineLen - 2))
	    {
	      PutKat (linebuf[Zeiger++]);	/* write rest of line to file   */
	    }			/* without eol and \0		*/
	}
      PutKat ((char) 0x0d);	/* write end of line marker	*/
    }				/* end of while			*/

  /* write end of file marker */
  PutKat ((char) 0x10);
  PutKat ((char) 0x20);
  PutKat ((char) 0x00);

  if (BlockP > 0)
    {
      size_t count;		/* clear unused part of block */
      for (count = BlockP; count <= 512; count++)
	{
	  KatBlock[count] = '\0';
	}
      (void) fwrite (KatBlock, 512, 1, OutFile);
    }

  (void) fseek (OutFile, 0, SEEK_SET);		/* goto begin of file	*/

  (void) fread (KatBlock, 512, 1, OutFile);	/* read first block	*/

  /* write lenth of ascii content into file */
#ifdef WORDS_BIGENDIAN
  KatBlock[4] = (char) ((FileP) & 0xFF);
  KatBlock[5] = (char) ((FileP >> 8) & 0xFF);
  KatBlock[6] = (char) ((FileP >> 16) & 0xFF);
  KatBlock[7] = (char) ((FileP >> 24));
#else
  KatBlock[7] = (char) ((FileP) & 0xFF);
  KatBlock[6] = (char) ((FileP >> 8) & 0xFF);
  KatBlock[5] = (char) ((FileP >> 16) & 0xFF);
  KatBlock[4] = (char) ((FileP >> 24));
#endif

  (void) fseek (OutFile, 0, SEEK_SET);		/* goto begin of file	*/

  (void) fwrite (KatBlock, 512, 1, OutFile);	/* override first block	*/
}

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
	  (void) fprintf (stderr, "NOTICE: Option %s is not implemented\n", "-u");
	  break;
      }	/* end of switch */

  if (errflg != 0)
    {
      (void) fprintf (stderr,
		      gettext ("usage: %s [-h] [-d] [-u] [-t tab_width] [-i inputfile] -o outputfile\n"),
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
    {
      fprintf (stderr, gettext ("\nCannot operate on stdout, output filename is required\n"));
      exit (EXIT_FAILURE);
    }
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

