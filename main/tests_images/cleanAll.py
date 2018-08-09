import os

if os.path.isfile("zresults.txt") : os.remove("zresults.txt")

for root, dirs, files in os.walk(".", topdown=False):
  
  for name in dirs:
  
    folder    = os.path.abspath(os.path.join(root, name))
    imageOut  = os.path.abspath(os.path.join(folder, "z_out.png"))
    imageOut2 = os.path.abspath(os.path.join(folder, "z_out.hdr"))
    imageOut3 = os.path.abspath(os.path.join(folder, "w_out.png"))          

    if os.path.isfile(imageOut)  : os.remove(imageOut)
    if os.path.isfile(imageOut2) : os.remove(imageOut2)
    if os.path.isfile(imageOut3) : os.remove(imageOut3)
    
print ("images were cleaned, all tests are ready to go!")
