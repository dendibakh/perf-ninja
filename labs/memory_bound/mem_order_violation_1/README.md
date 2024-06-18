This lab features an oldie-but-goodie Otsu's thresholding[^1] to convert a grayscale[^2] image to a binary[^3] image. One of the key parts in this algorithm is calculating a histogram of a grayscale image, i.e., calculate how many times a certain color appears in the image. Since the input image is 8-bit grayscale, there are only 256 different colors.

```cpp
std::array<uint32_t, 256> hist;
hist.fill(0);
for (int i = 0; i < image.width * image.height; ++i)
  hist[image.data[i]]++;
```

The implementation of the histogram algortihm is very simple but it has one nasty property. For each pixel on the image, you need to 1) read the current value of the corresponding color of the pixel, 2) increment it and 3) store it back.

When updates of the same color in the histogram occur at relatively high rates, the processor may not have completed updating pixel `i` prior to beginning pixel `i+1`. In such cases, a processor predicts whether the value loaded for the `i+1` update will come from memory or from the `i`'s store. If from memory, the two updates can be performed in parallel, otherwise the processor must serialize the updates.

Simple example: if you have the following pixels in the image:
```
0xFF 0xFF 0xFF 0xFF 0xFF 0xFF ...
```
Then all updates to `hist[0xFF]` will be serialized.

Think about how you can workaround this problem. Hint: you can use aditional memory.

Bonus exercise1: what would be the worst and the best cases for the original implementation and your solution?

Input images were taken from here: https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html

[^1]: https://en.wikipedia.org/wiki/Otsu%27s_method
[^2]: https://en.wikipedia.org/wiki/Grayscale
[^3]: https://en.wikipedia.org/wiki/Binary_image
