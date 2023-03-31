#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  Purpose:
#     kill an IFEM job by mpcci kill
#
#  $Id$
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;
use MpCCICore::Childs;
use MpCCICore::Textfile;

sub code_killer($$$)
{
   my ($codename,$xinpfile,$pid) = @_;
   my $solverName = $ENV{'_MPCCI_IFEM_SOLVER '}||''; # get the ifem solver


   mpcci_chdir(mpcci_dirname($xinpfile));

   child_system('mpcci','kill','-q','-r',$pid,$xinpfile,$solverName);   

}
1;
