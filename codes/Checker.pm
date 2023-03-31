#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# Purpose:
#   Check if the parameters' setup is correct.
#
# Author:
#     Carsten Brodbeck <carste.brodbeck@scai.fraunhofer.de>
#     Copyright (c) 2022, Fraunhofer Institute SCAI
#
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;
use MpCCIArch;
use MpCCICore::Which;
use MpCCICore::Couplinginfo;
use MpCCIGui::Env;
use MpCCIGui::Ccomp;
use MpCCIGui::Release;
use MpCCIGui::Codeinfo;
use MpCCICmds::Checker;

use POSIX;

my ($errors, $warnings) = (0, 0);



#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# The exported code_checker()
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sub code_checker($$@)
{
   my ($codeName,$modelFile) = @_;
    
   return ($errors, $warnings);
}
1;
