This lab calculates an image of [Mandelbrot set](https://en.wikipedia.org/wiki/Mandelbrot_set). In this algorithm, you can process each pixel independently, however, the number of iterations for processing each pixel can be different. The issue you will face when you process two pixels in a SIMD fashion: what to do when the processing loop for one pixel finishes after 100 iterations while for the adjacent pixel it runs for 200 iterations?

The `-ffast-math` option is disabled for validation purposes. It doesn't help compiler to autovectorize the code.

Lab assignment developed by Oleg Makovski (@0legmak).
