/* psbook.c
 * (c) Reuben Thomas 2012
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 *
 * rearrange pages in conforming PS file for printing in signatures
 */

#include "config.h"

#include <unistd.h>

#include "psutil.h"
#include "pserror.h"

char *program ;
int pages ;
int verbose ;
FILE *infile ;
FILE *outfile ;
char pagelabel[BUFSIZ] ;
int pageno ;

static void usage(void)
{
   fprintf(stderr, "%s %s\n", program, PACKAGE_VERSION);
   fprintf(stderr, COPYRIGHT_STRING);
   fprintf(stderr, "Usage: %s [-q] [-s<signature>] [infile [outfile]]\n",
	   program);
   fprintf(stderr, "       <signature> must be positive and divisible by 4\n");
   fflush(stderr);
   exit(1);
}


int
main(int argc, char *argv[])
{
   int signature = 0;
   int currentpg, maxpage;
   int opt;

   verbose = 1;
   program = *argv;

   while((opt = getopt(argc, argv, "vqs:")) != EOF) {
     switch(opt) {
     case 's':	/* signature size */
       signature = atoi(optarg);
       if (signature < 1 || signature % 4) usage();
       break;
     case 'q':	/* quiet */
       verbose = 0;
       break;
     case 'v':	/* version */
     default:
       usage();
       break;
     }
   }

   infile = stdin;
   outfile = stdout;

   /* Be defensive */
   if((argc - optind) < 0 || (argc - optind) > 2) usage();

   if (optind != argc) {
     /* User specified an input file */
     if ((infile = fopen(argv[optind], "rb")) == NULL)
       message(FATAL, "can't open input file %s\n", argv[optind]);
     optind++;
   }

   if (optind != argc) {
     /* User specified an output file */
     if ((outfile = fopen(argv[optind], "wb")) == NULL)
       message(FATAL, "can't open output file %s\n", argv[optind]);
     optind++;
   }

   if(optind != argc) usage();

   if ((infile=seekable(infile))==NULL)
      message(FATAL, "can't seek input\n");

   scanpages(NULL);

   if (!signature)
      signature = maxpage = pages+(4-pages%4)%4;
   else
      maxpage = pages+(signature-pages%signature)%signature;

   /* rearrange pages */
   writeheader(maxpage, NULL);
   writeprolog();
   writesetup();
   for (currentpg = 0; currentpg < maxpage; currentpg++) {
      int actualpg = currentpg - currentpg%signature;
      switch(currentpg%4) {
      case 0:
      case 3:
	 actualpg += signature-1-(currentpg%signature)/2;
	 break;
      case 1:
      case 2:
	 actualpg += (currentpg%signature)/2;
	 break;
      default: /* Avoid a compiler warning */
         break;
      }
      if (actualpg < pages)
	 writepage(actualpg);
      else
	 writeemptypage();
   }
   writetrailer();

   exit(0);
}
