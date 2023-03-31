#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
#  Purpose:
#     Contains the subroutine
#
#        code_subcommand()
#
#     which is dynamically loaded at runtime by the mpcci command.
#
#  Author:
#     Carsten Brodbeck <carsten.brodbeck@scai.fraunhofer.de>
#     Copyright (c) 2009-2022, Fraunhofer Institute SCAI
#
#  Reviews/changes:brodbe
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;
use MpCCICmds::Hash3ref;
use MpCCIGui::Codeinfo;

sub code_subcommand($)
{
   my $codeName = shift;

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# the hash table defined below translates an option into an environment variable
# or a code reference. Each hash entry is a reference to an array containing exact
# 3 values.
#  1) the MpCCI environment setup level 0=no, 1=arch 2=core, 3=gui
#  2) a short help message used in _help()
#  3) the name of an environment variable or a code reference
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
my %hash3; %hash3 = (
'-releases' =>[0,"[IFEM SOLVER NAME]
List all $codeName releases which MpCCI can find for a certain IFEM solver.",
   sub
   {
      $myName .= ' releases';

      for (@ARGV)
      {
         next if (/^(LinEl|ShellEl|IFEM-MpCCI)$/i) and $ENV{'_MPCCI_IFEM_SOLVER'} = $1;
         mpcci_die "$_: Invalid option.\n";
      }
      code_print_releases($codeName);
      @ARGV = ();
   }
],
'-info'     =>[0,"[IFEM SOLVER NAME]
List verbose information about all $codeName releases.",
   sub
   {
      $myName .= ' info';
      for (@ARGV)
      {
         next if (/^(LinEl|ShellEl|IFEM-MpCCI)$/i) and $ENV{'_MPCCI_IFEM_SOLVER'} = $1;
         mpcci_die "$_: Invalid option.\n";
      }
      code_print_info($codeName);
      @ARGV = ();
   }
],
'-scan'     =>[0,'<CASEDIR>
Run the scanner on the case and create a scanner output file.',
   sub
   {
      $myName .= ' scan';
      my $caseDir = shift(@ARGV);
      $caseDir = mpcci_getcwd() if (!$caseDir || $caseDir eq '.');
      require MpCCICmds::Scanner;
      scanner_run($codeName,$caseDir,1);
      @ARGV = ();
   }
],
'-help' =>
[
   0,'This screen.',
   sub
   {
      hash3ref_help(2,%hash3,"$myName [-]option","\'$myName\' is used to get information about $codeName.",0,'Options');
   }
]
);

   hash3ref_run('option',%hash3);
}
1;
