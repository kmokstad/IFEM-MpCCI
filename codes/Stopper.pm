#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  Purpose:
#     stop an IFEM job NOT YET KNOWN
#
#     $Id$
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCICore::Textfile;

sub code_stopper($$$)
{
   my ($codeName,$inpName) = @_;                    # get name of original input file

   textfile_save((IS_MSWIN) ? 'exit-ifem.txt' : 'exit-ifem');
   0;

}
1;
