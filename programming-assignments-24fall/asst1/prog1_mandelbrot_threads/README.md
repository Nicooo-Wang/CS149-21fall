1. Modify the starter code to parallelize the Mandelbrot generation using two processors. Specifically, compute the top half of the image in thread 0, and the bottom half of the image in thread 1. This type of problem decomposition is referred to as spatial decomposition since different spatial regions of the image are computed by different processors.  
修改启动代码以使用两个处理器并行生成 Mandelbrot 图像。具体来说，线程 0 计算图像的上半部分，线程 1 计算图像的下半部分。这种问题分解被称为空间分解，因为不同的处理器计算图像的不同空间区域。  
answer: NA

2. Extend your code to use 2, 3, 4, 5, 6, 7, and 8 threads, partitioning the image generation work accordingly (threads should get blocks of the image). Note that the processor only has four cores but each core supports two hyper-threads, so it can execute a total of eight threads interleaved on its execution contents. In your write-up, produce a graph of speedup compared to the reference sequential implementation as a function of the number of threads used FOR VIEW 1. Is speedup linear in the number of threads used? In your writeup hypothesize why this is (or is not) the case? (you may also wish to produce a graph for VIEW 2 to help you come up with a good answer. Hint: take a careful look at the three-thread datapoint.)  
扩展您的代码以使用 2、3、4、5、6、7 和 8 个线程，相应地划分图像生成工作（线程应获取图像的块）。请注意，处理器只有四个核心，但每个核心支持两个超线程，因此它可以在其执行内容上交错执行总共八个线程。在您的报告中，生成一个与参考顺序实现相比的加速图，作为使用线程数的函数，用于视图 1。加速与线程数的使用是否呈线性关系？在您的报告中，假设为什么（或不）是这样？（您还可以生成一个用于视图 2 的图表，以帮助您得出良好的答案。提示：仔细查看三线程数据点。）  
answer: 
| num of threads | speedup |
| -------------- | ------- |
| 1              | 1       |
| 2              | 1.99    |
| 3              | 1.65    |
| 4              | 2.42    |
| 5              | 2.46    |
| 6              | 3.08    |
| 7              | 3.18    |
| 8              | 3.07    |


3. To confirm (or disprove) your hypothesis, measure the amount of time each thread requires to complete its work by inserting timing code at the beginning and end of workerThreadStart(). How do your measurements explain the speedup graph you previously created?  
为了确认（或反驳）你的假设，通过在 workerThreadStart() 的开始和结束处插入计时代码，测量每个线程完成其工作所需的时间。你的测量结果如何解释你之前创建的速度提升图？  

```
[mandelbrot thread 0]:          [7.393] ms
[mandelbrot thread 6]:          [21.263] ms
[mandelbrot thread 2]:          [77.584] ms
[mandelbrot thread 4]:          [77.447] ms
[mandelbrot thread 1]:          [80.159] ms
[mandelbrot thread 5]:          [80.381] ms
[mandelbrot thread 3]:          [96.794] ms
[mandelbrot thread]:            [96.953] ms
Wrote image file mandelbrot-thread.ppm
                                (3.18x speedup from 7 threads)
```
显然，任务划分不均匀

4. Modify the mapping of work to threads to achieve to improve speedup to at about 7-8x on both views of the Mandelbrot set (if you're above 7x that's fine, don't sweat it). You may not use any synchronization between threads in your solution. We are expecting you to come up with a single work decomposition policy that will work well for all thread counts---hard coding a solution specific to each configuration is not allowed! (Hint: There is a very simple static assignment that will achieve this goal, and no communication/synchronization among threads is necessary.). In your writeup, describe your approach to parallelization and report the final 8-thread speedup obtained.  
修改工作到线程的映射以实现速度提升至大约 7-8 倍，在曼德布罗特集的两种视图上（如果你超过 7 倍也没关系，不用太担心）。你的解决方案中不得使用线程间的任何同步。我们期望你提出一个适用于所有线程数的单一工作分解策略——不允许针对每个配置硬编码特定的解决方案！（提示：有一个非常简单的静态分配可以实现这个目标，并且不需要线程间的通信/同步。）在你的论文中，描述你的并行化方法，并报告最终获得的 8 线程速度提升。  
 
5. Now run your improved code with 16 threads. Is performance noticably greater than when running with eight threads? Why or why not?  
现在运行您改进的代码，使用 16 个线程。性能是否明显优于使用 8 个线程时？为什么或为什么不？  