#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# Purpose/Coding rules/Usage: Please see Info.README
# Author:
#     Carsten Brodbeck <carsten.brodbeck@scai.fraunhofer.de>
#
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;
use MpCCIArch;
use MpCCICore::Which;
use MpCCIGui::Release;

my $rootKey    = '\\Software\\SINTEF\\IFEM'; # HARDPATH
my $exeName;
my $version;

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sub code_information # ($)
{
 #  return \%infoHash if (%infoHash);

   my $codeName = shift;
   my %infoHash = ();

# *******************************
# Release
# *******************************
# check if a solver was selected
   $exeName = $ENV{'_MPCCI_IFEM_SOLVER'}; 

   unless ($exeName)
   {
    mpcci_die
      "code_information: the $codeName solver name was not defined!\n"
   }   
  
# get solver executable
   my $addThisVersion=0;   
   for my $exe (where_executable($exeName))
   {
    $addThisVersion=0;   
    unless ($exe)
    {
     mpcci_die
       "$codeName solver executable for \"$exeName\" was not found!\n"
    }     

    if (0)
    {  
# get the version from a start with -check
     print"run $exe\n" if ($MPCCI_DEBUG);
   
     for (`$exe -check 2>&1`)
     {
#     print "$_\n";
     chomp;s|\s*$||g;
     if (/.*error.*/) { mpcci_die "$codeName could not successfully run solver \"$exe\":\n $_\n"}     
     if (/.*IFEM v(.*) initialized.*/) {$version = $1; next;}
     }
    }
    else
    {
# get the version(s) from the path
     if ($exe =~ /\/(\d+\.\d+\.\d+)\//)
     {
	 $version = $1;
	 $addThisVersion=1;
     }
     else
     {
          mpcci_warn
              "Cannot retrieve version for $codeName installation in\n",
               "   \"$exe\n\n" if($MPCCI_DEBUG);
     }
    }
    if ($addThisVersion)
    {
     my $pathToExecutable = $exe;
     my $codeDirectory    = mpcci_dirname($pathToExecutable);
     my $release          = $version;
     my $architecture     = mpcci_arch_base();
     my $adapterRelease   = 'STATIC';
     my $arch             = 'STATIC';
   
     print "infoHash($release):\n $pathToExecutable\n $codeDirectory\n $release\n $architecture\n $adapterRelease\n $arch\n\n" if ($MPCCI_DEBUG);

     $infoHash{$release} = [
       $codeName,
       $codeDirectory,
       $pathToExecutable,
       $release,
       $architecture,
       $adapterRelease,
       $arch
	 ];
    }
   }

   mpcci_die
      "Can\'t find any $codeName release installation for this architecture.\n"
       unless(%infoHash);
   
   return \%infoHash;
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

1;
