#!/usr/bin/env python
import os
import re
import subprocess
import argparse
import shutil
from astropy.io import fits 
<<<<<<< HEAD
import warnings

def pre_checks():
  locate = None
  locate = shutil.which('mpifxcorr')
  if (locate == None):
    print('mpifxcorr not found please make sure DiFX is installed')
    quit()
  locate = shutil.which('generateVDIF')
  if (locate == None):
    print('generateVDIF not found, please make sure it is installed.')  
    quit()

=======


parser = argparse.ArgumentParser()
parser.add_argument("--generateVDIF", help="Generate VDIF data? (yes/[no])",default="no")
parser.add_argument("--updatetest", help="Update the default test results (yes/[no])",default="no") 
input_args = parser.parse_args()
generateVDIF = input_args.generateVDIF
generateVDIF = generateVDIF.upper()
updatetest = input_args.updatetest
updatetest = updatetest.upper()
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)


def get_binary_files(directory):
  input_files = []
  contents = os.listdir(directory)

  difx_dirs = []
  for item in contents:
    if (item[-5:] == '.difx'):
      difx_dirs.append(item)
    if (item[-6:] == '.input'):
      path_and_input_file = directory + "/" + item
      input_files.append(path_and_input_file)
  difx_binary_files = []
  for difx_dir in difx_dirs:
    difx_dir = directory + "/" + difx_dir
    difx_dir_contents = os.listdir(difx_dir)
    for item in difx_dir_contents:
      if (item[:4] == 'DIFX'):
        path_and_file = difx_dir + "/" + item
        difx_binary_files.append(path_and_file)
  return difx_binary_files,input_files


def get_fits_file(directory):
  contents = os.listdir(directory)
  for item in contents:
    if (item[-5:] == '.FITS'):
      file_and_path = directory + item
      return file_and_path

def rm_output_files(testname):
  current_directory = os.getcwd()
  working_directory = current_directory + "/" + testname + "/"
  contents = os.listdir(working_directory)
  for item in contents:
     if (item[-4:] != '.v2d' and item[-4:] != '.vex' and item != 'machines' and item != 'benchmark_results'):
        fp_item = working_directory + item 
        if (item[-5:] == '.difx'):
          #print(fp_item)
          shutil.rmtree(fp_item)
        else:
          os.remove(fp_item)
  #quit()


def runtest(testname):
  current_directory = os.getcwd() 
  working_directory = current_directory + "/" + testname + "/"
<<<<<<< HEAD

  vex2difxlogfile = working_directory + "/vex2difx.log"
  vex2difxerrfile = working_directory + "/vex2difx.error"
  f_vex2difxlog = open(vex2difxlogfile,"w")
  f_vex2difxerr = open(vex2difxerrfile,"w")



  arg = "vex2difx test-" + testname + ".v2d"
  proc =subprocess.Popen(arg,cwd=working_directory,shell=True,stdout=f_vex2difxlog,stderr=f_vex2difxerr)
  proc.wait()

  difxcalclogfile = working_directory + "/difxcalc.log"
  difxcalcerrfile = working_directory + "/difxcalc.error"
  f_difxcalclog = open(difxcalclogfile,"w")
  f_difxcalcerr = open(difxcalcerrfile,"w")

  arg = "difxcalc test-" + testname + ".calc", 
  subprocess.Popen(arg,cwd=working_directory,shell=True,stdout=f_difxcalclog,stderr=f_difxcalcerr)
  proc.wait()
  arg = "mpirun -machinefile machines -np 4 mpifxcorr test-" + testname + ".input"
  difxlogfile = working_directory + "/mpifxcorr.log"
  difxerrfile = working_directory + "/mpifxcorr.error"
  f_difxlog = open(difxlogfile,"w")
  f_difxerr = open(difxerrfile,"w")
=======
  arg = "vex2difx test-" + testname + ".v2d"
  proc =subprocess.Popen(arg,cwd=working_directory,shell=True)
  proc.wait()

  arg = "difxcalc test-" + testname + ".calc", 
  subprocess.Popen(arg,cwd=working_directory,shell=True)
  proc.wait()
  arg = "mpirun -machinefile machines -np 4 mpifxcorr test-" + testname + ".input"
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)

  mpifxcorr_done = False
  # DiFX fails sometimes, re-run until it succeeds (need better fix here...)
  while (mpifxcorr_done == False):
<<<<<<< HEAD
    proc = subprocess.call(arg,cwd=working_directory,shell=True,stdout=f_difxlog,stderr=f_difxerr)
=======
    proc = subprocess.call(arg,cwd=working_directory,shell=True)
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)
    contents = os.listdir(working_directory)
    for item in contents: 
      if (item[-5:] == '.difx'):
        mpifxcorr_done = True
  #proc.wait()
<<<<<<< HEAD
  difx2fitslogfile = working_directory + "/difx2fits.log"
  difx2fitserrfile = working_directory + "/difx2fits.error"
  f_difx2fitslog = open(difx2fitslogfile,"w")
  f_difx2fitserr = open(difx2fitserrfile,"w")


  arg = "difx2fits test-" + testname
  proc = subprocess.Popen(arg,cwd=working_directory,shell=True,stdout=f_difx2fitslog,stderr=f_difx2fitserr)
=======
  arg = "difx2fits test-" + testname
  proc = subprocess.Popen(arg,cwd=working_directory,shell=True)
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)
  proc.wait()
 # quit()


<<<<<<< HEAD
def compare_results(testname, abstol, reltol):

  # results list [Binary file same as benchmark?,FITS file same as benchmark?]
  results = ["",True]
=======
def compare_results(testname):

  # results list [Binary file same as benchmark?,FITS file same as benchmark?]
  results = [True,True]
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)

  current_directory = os.getcwd()
  working_directory = current_directory + "/" + testname + "/"   
  #print(working_directory)
  binfiles,inputfiles = get_binary_files(working_directory)
  #print(binfiles)
  binfile = binfiles[0] 
  inputfile = inputfiles[0]
  results_directory = working_directory + "/benchmark_results/" 
  binfiles2,inputfiles2 = get_binary_files(results_directory) 
  benchmark_binfile = binfiles2[0]
  arg = "diffDiFX.py " + binfile + " " + benchmark_binfile + " " + "-i " + inputfile + " > binary_diff.log"   
  proc = subprocess.Popen(arg,cwd=working_directory,shell=True) 
  proc.wait()
  fits_file = get_fits_file(working_directory)
  fits_file_benchmark = get_fits_file(results_directory)
  #print(fits_file)
  hdu1 = fits.open(fits_file)
  hdu2 = fits.open(fits_file_benchmark) 
<<<<<<< HEAD

  warnings.resetwarnings()
  warnings.filterwarnings('ignore', category=UserWarning, append=True) 

  fd = fits.FITSDiff(hdu1, hdu2, ignore_keywords=['DATE-MAP','CDATE','HISTORY'], atol=abstol , rtol=reltol)  
  diff_fitslog = working_directory + "/fits_diff.log"  
  output = fd.report(fileobj=diff_fitslog, indent=0, overwrite=True) 
  results[1] = fd.identical
  hdu1.close()
  hdu2.close()
  #warnings.resetwarnings()
  #warnings.filterwarnings('always', category=UserWarning, append=True)
  #print("comparing fits_file = " + fits_file + " VS. benchmark fits file = " + fits_file_benchmark)
  #print(fd.identical)
  #print(output)

  fp_binary_diff_file = working_directory + 'binary_diff.log'
  with open(fp_binary_diff_file, 'r') as f:
    last_line = f.readlines()[-1]
  last_line = last_line.strip('\n')
  results[0] = last_line

=======
  fd = fits.FITSDiff(hdu1, hdu2, ignore_keywords=['DATE-MAP','CDATE','HISTORY'])  
  diff_fitslog = working_directory + "/fits_diff.log"  
  output = fd.report(fileobj=diff_fitslog, indent=0, overwrite=True) 
  results[1] = fd.identical
  print("comparing fits_file = " + fits_file + " VS. benchmark fits file = " + fits_file_benchmark)
  print(fd.identical)
  #print(output)
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)
  return(results)

def update_test(testname):
  current_directory = os.getcwd()
  working_directory = current_directory + "/" + testname + "/"
  results_directory = working_directory + "/benchmark_results/" 
  binfiles,inputfiles = get_binary_files(results_directory)
  results_binfile = binfiles[0]
  binfiles,inputfiles = get_binary_files(working_directory)
  current_binfile = binfiles[0]
  #print(results_binfile)
  #print(current_binfile)  
  fits_file = get_fits_file(working_directory)
  fits_file_benchmark = get_fits_file(results_directory) 
  #print()
  #print(fits_file)
  #print(fits_file_benchmark)
  os.remove(fits_file_benchmark)
  os.remove(results_binfile)
  args = "cp " + fits_file + " " + fits_file_benchmark
  subprocess.call(args,shell=True)
  #shutil.copy2(fits_file,fits_file_benchmark)
  shutil.copy2(current_binfile,results_binfile) 


def repackage_tests(testnames):
  current_directory = os.getcwd()
  tar_command = "tar -zcvf tests.tgz"
 
  for testname in testnames:
    working_directory = testname + "/"
    tar_command = tar_command + " " + working_directory + "/benchmark_results/*" + " " + working_directory + "test-" + testname + ".vex "+ working_directory + "test-" + testname + ".v2d " + working_directory + "machines"    
     #end for loop
  subprocess.call(tar_command,shell=True)


<<<<<<< HEAD
def unpackage_tests(testnames):
  unpack = False
  
  for testname in testnames:
    if (os.path.isdir(testname) == False):
      unpack = True
  if (unpack):
    subprocess.call("tar -zxvf tests.tgz",shell=True)

def create_test_data():
=======
if (generateVDIF == "YES"):
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)
  # Create Synthetic VDIF Data
  DURATION=10
  
  SEED1=38573
  SEED2=58573
<<<<<<< HEAD

  if (os.path.isdir("testdata") == False):
    os.mkdir("testdata")

  
  # USB Real 
  TEST1vdiflog = "testdata/TEST1.vdif.log"
  TEST1vdiferrfile = "testdata/TEST1.vdif.error"
  f_TEST1vdiflog = open(TEST1vdiflog,"w")
  f_TEST1vdiferrfile = open(TEST1vdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED1)+" -w 4 -b 2 -C 1 -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.5 -year 2020 -dayno 100 -time 07:00:00 testdata/TEST1.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST1vdiflog,stderr=f_TEST1vdiferrfile)


  TEST2_usbvdiflog = "testdata/TEST2-usb.vdif.log"
  TEST2_usbvdiferrfile = "testdata/TEST2-usb.vdif.error"
  f_TEST2_usbvdiflog = open(TEST2_usbvdiflog,"w")
  f_TEST2_usbvdiferrfile = open(TEST2_usbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l " + str(DURATION) + " -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 testdata/TEST2-usb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_usbvdiflog,stderr=f_TEST2_usbvdiferrfile)
  
  
  ## LSB Real
  TEST2_lsbvdiflog = "testdata/TEST2-lsb.vdif.log"
  TEST2_lsbvdiferrfile = "testdata/TEST2-lsb.vdif.error"
  f_TEST2_lsbvdiflog = open(TEST2_lsbvdiflog,"w")
  f_TEST2_lsbvdiferrfile = open(TEST2_lsbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l " + str(DURATION) + " -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -lsb testdata/TEST2-lsb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_lsbvdiflog,stderr=f_TEST2_lsbvdiferrfile)
  
  
  ## Complex (single side band)
  TEST1_complex_usbvdiflog = "testdata/TEST1-complex-usb.vdif.log"
  TEST1_complex_usbvdiferrfile = "testdata/TEST1-complex-usb.vdif.error"
  f_TEST1_complex_usbvdiflog = open(TEST1_complex_usbvdiflog,"w")
  f_TEST1_complex_usbvdiferrfile = open(TEST1_complex_usbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED1)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.5 -year 2020 -dayno 100 -time 07:00:00 -complex testdata/TEST1-complex-usb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST1_complex_usbvdiflog,stderr=f_TEST1_complex_usbvdiferrfile)

  TEST2_complex_usbvdiflog = "testdata/TEST2-complex-usb.vdif.log"
  TEST2_complex_usbvdiferrfile = "testdata/TEST2-complex-usb.vdif.error"
  f_TEST2_complex_usbvdiflog = open(TEST2_complex_usbvdiflog,"w")
  f_TEST2_complex_usbvdiferrfile = open(TEST2_complex_usbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert testdata/TEST2-complex-usb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_complex_usbvdiflog,stderr=f_TEST2_complex_usbvdiferrfile)
  
  TEST2_complex_lsbvdiflog = "testdata/TEST2-complex-lsb.vdif.log"
  TEST2_complex_lsbvdiferrfile = "testdata/TEST2-complex-lsb.vdif.error"
  f_TEST2_complex_lsbvdiflog = open(TEST2_complex_lsbvdiflog,"w")
  f_TEST2_complex_lsbvdiferrfile = open(TEST2_complex_lsbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -lsb testdata/TEST2-complex-lsb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_complex_lsbvdiflog,stderr=f_TEST2_complex_lsbvdiferrfile)
  


  ## Complex (double side band)
  TEST2_dsb_usbvdiflog = "testdata/TEST2-dsb-usb.vdif.log"
  TEST2_dsb_usbvdiferrfile = "testdata/TEST2-dsb-usb.vdif.error"
  f_TEST2_dsb_usbvdiflog = open(TEST2_dsb_usbvdiflog,"w")
  f_TEST2_dsb_usbvdiferrfile = open(TEST2_dsb_usbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -doublesideband testdata/TEST2-dsb-usb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_dsb_usbvdiflog,stderr=f_TEST2_dsb_usbvdiferrfile)



  TEST2_dsb_lsbvdiflog = "testdata/TEST2-dsb-lsb.vdif.log"
  TEST2_dsb_lsbvdiferrfile = "testdata/TEST2-dsb-lsb.vdif.error"
  f_TEST2_dsb_lsbvdiflog = open(TEST2_dsb_lsbvdiflog,"w")
  f_TEST2_dsb_lsbvdiferrfile = open(TEST2_dsb_lsbvdiferrfile,"w")
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -doublesideband -lsb testdata/TEST2-dsb-lsb.vdif"
  subprocess.call(args,shell=True,stdout=f_TEST2_dsb_lsbvdiflog,stderr=f_TEST2_dsb_lsbvdiferrfile)
  
  f_TEST1vdiflog.close()
  f_TEST1vdiferrfile.close()
  f_TEST2_usbvdiflog.close()
  f_TEST2_usbvdiferrfile.close()
  f_TEST2_lsbvdiflog.close()
  f_TEST2_lsbvdiferrfile.close()
  f_TEST1_complex_usbvdiflog.close()
  f_TEST1_complex_usbvdiferrfile.close()
  f_TEST2_complex_usbvdiflog.close()
  f_TEST2_complex_usbvdiferrfile.close()
  f_TEST2_complex_lsbvdiflog.close()
  f_TEST2_complex_lsbvdiferrfile.close()
  f_TEST2_dsb_usbvdiflog.close()
  f_TEST2_dsb_usbvdiferrfile.close()
  f_TEST2_dsb_lsbvdiflog.close()
  f_TEST2_dsb_lsbvdiferrfile.close()


def display_test_results(passfail):

  print()
  print()
  print("FITS FILE COMPARISON TEST RESULTS:")
  print()
  for testname in passfail:
    fits_result = "FAIL"
    if (passfail[testname][1]): fits_result = "PASS"
    string = "{0:20}{1}".format(testname, fits_result)
    print(string)

  print()
  print()
  print("BINARY FILE COMPARISON TEST RESULTS:")
  print()
  for testname in passfail:
    binary_result = passfail[testname][0]
    string = "{0:20}{1}".format(testname,binary_result)
    print(string)
  print()
  print()
  print('For more information please inspect the full log files located within each test directory *testname*/binary_diff.log  *testname*/fits_diff.log') 
  
  


def main():

  parser = argparse.ArgumentParser()
  parser.add_argument("--generateVDIF", help="Generate VDIF data? (yes/[no])",default="no")
  parser.add_argument("--updatetest", help="Update the default test results (yes/[no])",default="no") 
  parser.add_argument("--abstol",help="Update the absolute tolerance for FITS file comparison (default = 1e-6)",default=1e-6)
  parser.add_argument("--reltol",help="relative tolerance for FITS file comparison (default = 1e-6)",default=1e-6)

  input_args = parser.parse_args()
  generateVDIF = input_args.generateVDIF
  generateVDIF = generateVDIF.upper()
  updatetest = input_args.updatetest
  updatetest = updatetest.upper()
  reltol = float(input_args.reltol)
  abstol = float(input_args.abstol)

  pre_checks()

  # list of test names
  test_name_list = ["complex-complex","lsb","lsb-complex","lsb-dsb","usb","usb-complex","usb-dsb"]
  # Dictionary with pass/fail status of each test {"test name" : [binary file same (results), FITS file same (true/false)]} 
  passfail = {"complex-complex":["",True],"lsb":["",True],"lsb-complex":["",True],"lsb-dsb":["",True],"usb":["",True],"usb-complex":["",True],"usb-dsb":["",True]}

  # Upack tests if they haven't been already
  unpackage_tests(test_name_list)
  #quit()
  # Generate synthetic VDIF data
  if (generateVDIF == "YES" or (os.path.isdir("testdata") == False)):
    create_test_data()
   
  # Run DiFX on  
  for testname in test_name_list:
    rm_output_files(testname)
    runtest(testname)
  
  # Run comparison with the results
  for testname in test_name_list:
    passfail[testname] = compare_results(testname,abstol,reltol)
  
  if (updatetest == "YES"):
    for testname in test_name_list:
      update_test(testname)
    repackage_tests(test_name_list)

  display_test_results(passfail)

if __name__ == "__main__":
    main()
=======
  
  # USB Real 
  args = "generateVDIF -seed="+str(SEED1)+" -w 4 -b 2 -C 1 -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.5 -year 2020 -dayno 100 -time 07:00:00 testdata/TEST1.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l " + str(DURATION) + " -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 testdata/TEST2-usb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  
  
  ## LSB Real
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l " + str(DURATION) + " -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -lsb testdata/TEST2-lsb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  
  
  ## Complex (single side band)
  args = "generateVDIF -seed="+str(SEED1)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.5 -year 2020 -dayno 100 -time 07:00:00 -complex testdata/TEST1-complex-usb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert testdata/TEST2-complex-usb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert testdata/TEST2-complex-usb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -lsb testdata/TEST2-complex-lsb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  
  ## Complex (double side band)
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -doublesideband testdata/TEST2-dsb-usb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
  args = "generateVDIF -seed="+str(SEED2)+" -w 4 -b 2 -C 1  -l "+str(DURATION)+" -noise -amp2 0.05 -tone2 1.0 -year 2020 -dayno 100 -time 07:00:00 -hilbert -doublesideband -lsb testdata/TEST2-dsb-lsb.vdif"
  #print(args)
  subprocess.call(args,shell=True)
# end if (generateVDIF == "YES"):  


# Run test correlations
test_name_list = ["complex-complex","lsb","lsb-complex","lsb-dsb","usb","usb-complex","usb-dsb"]

# Dictionary with pass/fail status of each test {"test name" : [binary file same (true/false), FITS file same (true/false)]}
passfail = {"complex-complex":[True,True],"lsb":[True,True],"lsb-complex":[True,True],"lsb-dsb":[True,True],"usb":[True,True],"usb-complex":[True,True],"usb-dsb":[True,True]}

#  
for testname in test_name_list:
  rm_output_files(testname)
  runtest(testname)

# Run comparison with the results
for testname in test_name_list:
  compare_results(testname)

if (updatetest == "YES"):
  for testname in test_name_list:
    update_test(testname)
  repackage_tests(test_name_list)

#repackage_tests(test_name_list)
>>>>>>> 3d7c666ee (Adding unit testing utility for difx)



