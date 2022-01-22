**TODO**: add an introductory and a summary videos.

This lab assignment focuses on improving performance by reducing the number of branch target mispredictions. In this lab,
we have a collection of objects of three distinct classes that inherit from a shared base class. An array of objects is
created in some arbitrary order and then repeatedly iterated. During the iteration, a virtual method is called on each
object. The CPU does not know which exact method will be called beforehand (it does not know the *target* of the call),
which slows its execution down.

Your task here is to reduce the number of branch target mispredictions by making the virtual method calls more predictable.

Authored-by: Jakub Beránek (@Kobzol)