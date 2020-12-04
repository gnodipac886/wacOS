from scipy import misc
import numpy as np
import imageio

s = input("Please type file name here(with .png): ")
name = s.split('.png')[0]
arr = np.array(imageio.imread(s))

rgb_565 = np.array([], dtype=np.uint16)
# Red - 5, Green - 6, Blue - 5
# RRRRRGGGGGGBBBBB
for i in range(arr.shape[0]):
    for j in range(arr.shape[1]):
        red = (arr[i, j, 0] >> 3) << 11
        green = (arr[i, j, 1] >> 2) << 5
        blue = (arr[i, j, 2] >>3)

        rgb_565 = np.append(rgb_565, np.uint16(red | green | blue))

np.array(rgb_565).astype('uint16').tofile(name + ".bin")
