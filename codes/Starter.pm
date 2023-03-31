#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
# Purpose:
#     IFEM code starter
#
# Author:
#     Carsten Brodbeck <carsten.brodbeck@scai.fhg.de>
#     Copyright (c) 2010-2022, Fraunhofer Institute SCAI
#
#     $Id$
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
use strict;
use MpCCIBase;
use MpCCICore::Which;
use MpCCIGui::Env;
use MpCCIGui::Codeinfo;
use MpCCICore::Couplinginfo;
use MpCCIGui::Codestart;
use MpCCICore::Textfile;
use MpCCICore::Hostlist;

sub code_starter($$@)
{
   my $codeName = shift;
   my $xinpFile = shift; 
   my @argv;
   my $ifemPetsc="";
   my $ompVar="OMP_NUM_THREADS";
   my $ifemNumThreads=0;
       
   my (undef,undef,$exe) = code_info($codeName);
   
   # ifem solver name selected
   my $ifemSolver   = env_required_value('_MPCCI_IFEM_SOLVER');
   
   # ifem additional command line options
   my $ifemOptions  = env_optional_value('_MPCCI_IFEM_CMDOPTS');

   #--------------------------------------------
   # parallel thread based if activated in gui
   my $ifemHasThreads = env_boolean_value('_MPCCI_IFEM_PARA_RUNTH');

   if ($ifemHasThreads)
   {
       $ifemNumThreads = env_required_value('_MPCCI_IFEM_PARA_NTHREADS');
       mpcci_msg "Set $ompVar env variable to $ifemNumThreads\n" if ($MPCCI_DEBUG);
       $ENV{$ompVar}= $ifemNumThreads;
   }
   
   #--------------------------------------------
   # mpi based parallel run if activated in gui
   my $ifemIsParallel = env_boolean_value('_MPCCI_IFEM_PARA_RUN');

   if ($ifemIsParallel)
   {
     mpcci_msg "Set parallel environment for MPI\n" if ($MPCCI_DEBUG);
       
     # always add -petsc as argument for ifem if mpi activated
     $ifemPetsc ="-petsc";

     # prepare hostlist
     my ($numProcs,undef,@hostList) = code_start_prepare($codeName,0,1,1);	   
     my $mpirunExe = get_executable('mpirun');
     my $hostfile    = (@hostList > 1)
        ? textfile_save('mpcci_ifem_hosts_file.txt',hostlist_join(\@hostList,"\n"))
	 : undef;

     # fill arg with mpirun options
     @argv = ($mpirunExe,'-np',$numProcs);
     push(@argv,'-x',$ompVar)          if ($ifemHasThreads);
     push(@argv,'-hostfile',$hostfile) if ($hostfile);

     # patch the .xinp file and insert a file name for log files
     my $ifemJobName  = env_optional_value('_MPCCI_IFEM_JOBNAME');
     _patch_xinp_log($xinpFile,$ifemJobName);
   }
      

   # push all other arguments on argv
   push(@argv,$exe);
   push(@argv,$xinpFile);
   push(@argv,$ifemOptions);
   if ($ifemPetsc){push(@argv,$ifemPetsc);}
   
#   print"@argv\n";
   mpcci_msg "@argv\n" if ($MPCCI_DEBUG);    

   my %infoHash =
   (
      'STDOUT'  => 'mpcci',        # valid: [xterm|mpcci|null] [file]
      'STDERR'  => 'mpcci',        # valid: [xterm|mpcci|null] [file]
#      'STDLOG'  => $ifemLogFile,   # valid: string
      'BACKGR'  => 'azure',        # only used with: xterm|mpcci'
      'FOREGR'  => 'black'         # only used with: xterm|mpcci'
   );

   return (\@argv,\%infoHash);
}

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# we patch the .xinp file with a prefix for logfiles for each mpi process

sub _patch_xinp_log($$)
{
    my $xinpInputFile = shift;
    my $prefix        = shift;
    my $consoleExists = 0;

    # prefix is empty do nothing
    unless ($prefix)
    {
	mpcci_msg "Prefix empty\n" if ($MPCCI_DEBUG);
	return;
    }

    mpcci_msg "Patching the input file \"$xinpInputFile\" with logging to \"$prefix\"\n" if ($MPCCI_DEBUG);

    #---------------   
    # read the file and check content
    #---------------
    my @xinpInputText = textfile_load($xinpInputFile);

    # check if console was already added
    foreach my $i (@xinpInputText)
    {
	if ($i =~ /<console>/)
	{
	    $consoleExists=1;
        }
	#check if it is the same prefix
	if ($i =~ /logging\s+output_prefix=\"(.*)\"/)
        {
	    if ($1 eq $prefix)
	    {
		#jump out nothing to be done
	        mpcci_msg "Prefix \"$prefix\" already in file!\n" if ($MPCCI_DEBUG);
		return;
	    } 
	}
    }

    #---------------
    # write to file
    #---------------
    my $patchedFile  = textfile_wopen($xinpInputFile);
	
    foreach my $i (@xinpInputText)
    {
	if ($i =~ /logging\s+output_prefix=\"(.*)\"/)
	{
	    mpcci_msg "Replace prefix \"$prefix\".\n" if ($MPCCI_DEBUG);
	    # already there replace name
	    $i =~ s/$1/$prefix/;
	    unless ($consoleExists){mpcci_msg "Something went wrong\n";}
	}
	
	# write everything which was there, maybe modified prefix
	print $patchedFile "$i";

	# right after siumlation block started
	if ($i =~ /<simulation>/)
	{
	    unless($consoleExists)
	    {
		# no console add it
		mpcci_msg "Add prefix \"$prefix\".\n" if ($MPCCI_DEBUG);
	        print $patchedFile " <console>\n";
	        print $patchedFile "  <logging output_prefix=\"$prefix\"\/>\n";
	        print $patchedFile " </console>\n";
	    }
	}


    }
    close($patchedFile);
}
1;
