***********************

libOpenCL.so:
sudo apt install ocl-icd-opencl-dev

***********************

NVidia GPU:
should be included with drivers, do nothing ... but on pure Debian you probably need to install:
sudo apt install nvidia-opencl-icd

***********************

AMD GPU: 
should be included with drivers, do nothing

***********************

Intel CPU:
download https://software.intel.com/en-us/articles/opencl-drivers#latest_CPU_runtime

sudo ./install_GUI.sh

***********************

OpenCL headers (if needed):

as admin:

TGT_DIR=/opt/opencl-headers/include/CL  
mkdir -p $TGT_DIR && cd $TGT_DIR
wget https://raw.githubusercontent.com/KhronosGroup/OpenCL-Headers/master/opencl22/CL/{opencl,cl_platform,cl,cl_ext,cl_gl,cl_gl_ext}.h

***********************

Testing 1:

curl https://codeload.github.com/hpc12/tools/tar.gz/master | tar xvfz -
cd tools-master
make

./print-devices
./cl-demo 1000000 10

***********************

Testing 2:

sudo apt-get install clinfo

clinfo

***********************

link your application against -lOpenCL



