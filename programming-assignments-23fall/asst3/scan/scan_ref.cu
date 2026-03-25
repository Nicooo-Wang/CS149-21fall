// CUDA Scan (Parallel Prefix Sum) Implementation
// 这是一个并行前缀和（Prefix Sum）算法的 CUDA 实现

// 定义每个 block 最多 1024 个线程
#define MAX_THREADS_PER_BLOCK 1024
// 定义每个 block 处理 2 * 1024 = 2048 个元素（每个线程处理2个元素）
#define MAX_ELEMENTS_PER_BLOCK (MAX_THREADS_PER_BLOCK * 2)

// 并行大规模扫描内核
// data: 输入数据数组
// prefix_sum: 输出前缀和数组
// N: 输入数据总数
// sums: 存储每个 block 的累计和（用于多级扫描）
__global__ void parallel_large_scan_kernel(int *data, int *prefix_sum, int N, int *sums)
{
    // 声明共享内存，大小为每个 block 能处理的最大元素数
    __shared__ int tmp[MAX_ELEMENTS_PER_BLOCK];
    
    // 获取当前线程在 block 内的索引（0 到 1023）
    int tid = threadIdx.x;
    // 获取当前 block 的索引
    int bid = blockIdx.x;
    // 计算当前 block 在全局数组中的偏移量
    int block_offset = bid * MAX_ELEMENTS_PER_BLOCK;
    // 每个 block 处理的叶子节点数（也是 tmp 数组的大小）
    int leaf_num = MAX_ELEMENTS_PER_BLOCK;

    // 将数据加载到共享内存tmp数组中
    // 每个线程处理两个元素：tid*2 和 tid*2+1
    // 如果超出数组范围，则填充0
    tmp[tid * 2] = tid * 2 + block_offset < N ? data[tid * 2 + block_offset] : 0;
    tmp[tid * 2 + 1] = tid * 2 + 1 + block_offset < N ? data[tid * 2 + 1 + block_offset] : 0;
    
    // 同步，确保所有线程都完成数据加载
    __syncthreads();

    // === 向上归约阶段（Up-sweep / Reduction Phase） ===
    // 计算前缀和，从叶子节点向上计算
    int offset = 1;
    // 从 leaf_num/2 开始，每次折半，直到只剩1个元素
    for (int d = leaf_num >> 1; d > 0; d >>= 1)
    {
        // 每个线程处理一对节点：父节点的左右子节点
        if (tid < d)
        {
            // 计算左子节点索引（ai）和右子节点索引（bi）
            int ai = offset * (2 * tid + 1) - 1;
            int bi = offset * (2 * tid + 2) - 1;
            // 将左子节点的值加到右子节点上（Hill-Steele算法）
            tmp[bi] += tmp[ai];
        }
        // 更新 offset，为下一轮迭代做准备
        offset *= 2;
        // 同步，确保所有线程完成本轮计算后再继续
        __syncthreads();
    }

    // === 处理根节点 ===
    // 将最后一个元素（即整个 block 的总和）保存到 sums 数组中
    // 然后将根节点置为0，作为下一阶段（向下扫描）的起点
    if (tid == 0)
    {
        sums[bid] = tmp[leaf_num - 1];  // 保存 block 的累计和
        tmp[leaf_num - 1] = 0;           // 将根节点置0
    }
    // 同步
    __syncthreads();

    // === 向下扫描阶段（Down-sweep Phase） ===
    // 从根节点向下计算最终的前缀和
    for (int d = 1; d < leaf_num; d *= 2)
    {
        // 反向更新 offset（从根向下）
        offset >>= 1;
        if (tid < d)
        {
            // 再次计算左右子节点索引
            int ai = offset * (2 * tid + 1) - 1;
            int bi = offset * (2 * tid + 2) - 1;

            // 保存左子节点的当前值
            int v = tmp[ai];
            // 将右子节点的值加到左子节点上
            tmp[ai] = tmp[bi];
            // 右子节点 = 原左子节点的值 + 原右子节点的值
            tmp[bi] += v;
        }
        // 同步
        __syncthreads();
    }

    // === 写回结果 ===
    // 将计算好的前缀和写回到全局 prefix_sum 数组
    // 注意：这里实现的是 exclusive scan（前缀和不包含当前元素本身）
    if (tid * 2 + block_offset < N)
    {
        prefix_sum[tid * 2 + block_offset] = tmp[tid * 2];
    }
    if (tid * 2 + 1 + block_offset < N)
    {
        prefix_sum[tid * 2 + 1 + block_offset] = tmp[tid * 2 + 1];
    }
}

// 添加累计和的内核函数
// 用于在多级扫描后，将每个 block 的前缀和加到对应的结果上
// prefix_sum: 输入/输出的前缀和数组（每个block内部已计算好）
// valus: 每个block的累计和数组（即 sums 数组的前缀和）
// N: 总元素数
__global__ void add_kernel(int *prefix_sum, int *valus, int N)
{
    // 获取线程索引
    int tid = threadIdx.x;
    int bid = blockIdx.x;
    // 计算当前 block 在全局数组中的偏移量
    int block_offset = bid * MAX_ELEMENTS_PER_BLOCK;
    // 计算当前 block 内两个"半区"的起始位置
    // ai: 前半区（0 到 1023）
    // bi: 后半区（1024 到 2047）
    int ai = tid + block_offset;
    int bi = tid + (MAX_ELEMENTS_PER_BLOCK >> 1) + block_offset;

    // 如果前半区索引在有效范围内，将对应 block 的累计和加到每个元素上
    if (ai < N)
    {
        prefix_sum[ai] += valus[bid];
    }
    // 如果后半区索引在有效范围内，同样加上累计和
    if (bi < N)
    {
        prefix_sum[bi] += valus[bid];
    }
}

// 递归扫描函数（处理大规模数据的分层扫描）
// d_data: 输入数据设备指针
// d_prefix_sum: 输出前缀和设备指针
// N: 元素总数
void recursive_scan(int *d_data, int *d_prefix_sum, int N)
{
    // 计算处理所有数据需要的 block 数量
    // 每个 block 能处理 MAX_ELEMENTS_PER_BLOCK (2048) 个元素
    int block_num = N / MAX_ELEMENTS_PER_BLOCK;
    // 如果不能整除，需要额外一个 block
    if (N % MAX_ELEMENTS_PER_BLOCK != 0)
    {
        block_num += 1;
    }
    
    // 分配设备内存：
    // d_sums: 保存每个 block 的累计和
    // d_sums_prefix_sum: 保存 block 累计和的前缀和
    int *d_sums, *d_sums_prefix_sum;  // 用来保存block数组和、数组和的前缀和
    cudaMalloc(&d_sums, block_num * sizeof(int));
    cudaMalloc(&d_sums_prefix_sum, block_num * sizeof(int));

    // 第一步：对每个 block 独立进行扫描
    // 每个 block 计算自己范围内的前缀和，并将累计和保存到 d_sums
    parallel_large_scan_kernel<<<block_num, MAX_THREADS_PER_BLOCK>>>(d_data, d_prefix_sum, N, d_sums);

    // 第二步：如果有多个 block，递归处理累计和数组
    // 这是一个分治策略：将大问题分解为小问题
    if (block_num != 1)
    {
        // 递归对 d_sums 计算前缀和，结果保存在 d_sums_prefix_sum
        recursive_scan(d_sums, d_sums_prefix_sum, block_num);
        
        // 第三步：将 block 级别的累计和前缀和加回到原始结果中
        // 这样每个 block 内的元素都会加上前面所有 block 的累计和
        add_kernel<<<block_num, MAX_THREADS_PER_BLOCK>>>(d_prefix_sum, d_sums_prefix_sum, N);
    }
}
