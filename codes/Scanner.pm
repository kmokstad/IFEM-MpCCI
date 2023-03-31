#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# Purpose:
#     MpCCI IFEM Master scanner
#
# Author:
#     Carsten Brodbeck <carsten.brodbeck@scai.fhg.de>
#     Copyright (c) 2012-2022, Fraunhofer Institute SCAI
#
# Reviews/changes:
#     $Id$
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;

my $codeName;

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# Scan the input file. Either a .cfg input file for the EAS Master or a single FMU (.fmu)
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sub code_scanner($$$)
{
   my $xinpFile;
   ($codeName, $xinpFile) = @_;
   #my (undef,$home,undef,$release,$arch) = code_info($codeName);
   my %regionList;
   
   my $solutionType   = 'S';    # default static
   my $modelDimension = '3D';   # default 3D
#   my %regionCounter;
#   $regionCounter['vertex']  = 0;
#   $regionCounter['edge']    = 0;
#   $regionCounter['face']    = 0;
#   $regionCounter['surface'] = 0;
   my $regionCounter=1;
   my $handleSrc = textfile_ropen($xinpFile,"$codeName config file");
   while (my $line = <$handleSrc>)
   {
       if( $line =~ /<timestepping>/          ){ $solutionType   = 'T'; }
       elsif( $line =~ /<geometry.*dim=\"2\"/i   ){ $modelDimension = '2D'; }
       elsif( $line =~ /<geometry.*dim=\"1\"/i   ){ $modelDimension = '1D'; }
       elsif( $line =~ /<set\s+name=\"(\S+)\"\s+type=\"(\S+)\"/i)
       {
	   my $setName = $1;
	   my $setType = $2;
	   my $setID = $regionCounter;
	   $regionCounter++;
	   $regionList{$setName} = [$setName,$setType,$setID];
       }
   };
   close($handleSrc);
   
#-------------------------   
# modelInfo hash
#-------------------------   
   my %modelInfo =
   (
      MDIM => $modelDimension,
      CSYS => 'C',
      SOLU => $solutionType,
      LCAS => 1,
      UNIT => 'SI',
      PREC => 'D'
   );
   
   my %userInfo =
   (
#       'TODO'   => 'TODOr'
   );
   
   my %varHash =
   (
#      'ScannerInfo.nsim' => $nsim,
#      'GoMenuEntries.RuntimeConfig.tstart' => $tstart,
#      'GoMenuEntries.RuntimeConfig.tend' => $tend,
#      'GoMenuEntries.RuntimeConfig.tstepmax' => $tstepmax,
#      'GoMenuEntries.RuntimeConfig.tstepstart' => $tstepstart,
#      'GoMenuEntries.RuntimeConfig.masterdebug' => $masterdebug,
#      'GoMenuEntries.RuntimeConfig.CoSimulation.outputgnuplot' => $outputgnuplot,
#      'GoMenuEntries.MpCCITinfo.IterationControl.iter_max' => $it_max_steps,
#      'GoMenuEntries.RuntimeConfig.CoSimulation.it_tol_abs' => $it_tol_abs,
#      'GoMenuEntries.RuntimeConfig.CoSimulation.it_tol_rel' => $it_tol_rel
   );

   return (\%regionList,\%modelInfo,\%userInfo,\%varHash);
}
1;
