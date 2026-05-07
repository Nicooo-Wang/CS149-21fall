#include <torch/extension.h>
#include <ATen/ATen.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <immintrin.h>

// Uncomment for ISPC
//#include "module_ispc.h"
//using namespace ispc;
int g_zero = 0;

// ------------------------------------ //
// 	WARM-UP: ACCESSING TENSORS      //
// ------------------------------------ //

// Step #1: Understand Read/Write Accessors for a 2D Tensor
inline float twoDimRead(std::vector<float> &tensor, int &x, int &y, const int &sizeX) {
    // Note that sizeX is the size of a Row, not the number of rows
    return tensor[x * (sizeX)+ y];
}

inline void twoDimWrite(std::vector<float> &tensor, int &x, int &y, const int &sizeX, float &val) {
    tensor[x * (sizeX) + y] = val;
}

// Step #2: Implement Read/Write Accessors for a 4D Tensor
inline float fourDimRead(std::vector<float> &tensor, int &x, int &y, int &z, int &b,
        const int &sizeX, const int &sizeY, const int &sizeZ) {
    size_t offset = x * (sizeX * sizeY * sizeZ) + y * (sizeY * sizeZ) + z * sizeZ + b;
    return tensor[offset];
}

inline void fourDimWrite(std::vector<float> &tensor, int &x, int &y, int &z, int &b,
        const int &sizeX, const int &sizeY, const int &sizeZ, float &val) {
    size_t offset = x * (sizeX * sizeY * sizeZ) + y * (sizeY * sizeZ) + z * sizeZ + b;
    tensor[offset] = val;
}

// DO NOT EDIT THIS FUNCTION //
std::vector<float> formatTensor(torch::Tensor tensor) {
    tensor = tensor.flatten();
    tensor = tensor.contiguous();
    std::vector<float> vec(tensor.data_ptr<float>(), tensor.data_ptr<float>() + tensor.numel());
    return vec;
}

/* Programming Your Attention Modules.
 * 
 * You are given Q, K, and V Tensors as inputs that are formatted as vectors. We have also created O and QK^t Tensors 
 * that are formatted as vectors. After you have implemented your accessors in the Warm-Up you should be able to
 * read/write to these tensors via the read/write functions above.
 *
 * You are also given 4 integers as parameters: B, H, N, d:
 *
 * B (Batch Size) - The number of samples for your attention layer. Think of it this way - if I asked my dnn
 * a question and it output 5 different answers it had a batch size of 5. These samples are independent of each
 * other and thus can be parallelized.
 *
 * H (Number of Heads) - Each head runs on its own set of Q, K, V matrices. This effectively allows each head
 * to operate the same attention algorithm, but each with each head using different hyperparameters. These
 * allow each head to have their own definition of what relevance is when looking at a token. These heads
 * can operate independently of one another and thus can be parallized.
 *
 * N (Sequence Length) - The number of tokens. You may think of this as the number of words in a sample.
 *
 * d (Embedding Dimensionality) - The number of features each token encodes per attention head. Let's
 * say I encoded a word using the follow (length, number of vowels, has a capital letters). The
 * emvedded dimensionaliy would be 3.
 * */

// ---------------------------------------------------------- //
//                  PART 1: NAIVE ATTENTION                   //
// ---------------------------------------------------------- //

bool MatmulTranspose4Dto2D(std::vector<float> &Q, std::vector<float> &K_t, std::vector<float> &QK_t,
                           std::vector<int> QDims, std::vector<int> K_tDims, int selectB, int selectH, int mStart,
                           int mEnd, int nStart, int nEnd)
{
    if (QDims.size() != 4 || K_tDims.size() != 4 ) {
        std::cerr << "Invalid dimensions for Q, K^t." << std::endl;
        return false;
    }
    if (QDims[0] != K_tDims[0] || QDims[1] != K_tDims[1] || QDims[3] != K_tDims[3] || QDims[2] != K_tDims[2])
    {
        std::cerr << "Batch size, number of heads, and embedding dimensionality must match for Q and K^t." << std::endl;
        std::cerr << "Q Dims: " << QDims[0] << " " << QDims[1] << " " << QDims[2] << " " << QDims[3] << std::endl;
        std::cerr << "K^t Dims: " << K_tDims[0] << " " << K_tDims[1] << " " << K_tDims[2] << " " << K_tDims[3] << std::endl;
        return false;
    }
     
    int B = QDims[0];
    int H = QDims[1];
    int M = QDims[2];
    int N = QDims[2];
    int K = QDims[3];

    const int K_TILE = 32;

    // Zero the output tile
    for (int m = mStart; m < mEnd; m++)
        for (int n = nStart; n < nEnd; n++) {
            float zero = 0.0f;
            twoDimWrite(QK_t, m, n, N, zero);
        }

    // K-tiled accumulation
    for (int k = 0; k < K; k += K_TILE) {
        int kEnd = std::min(k + K_TILE, K);
        for (int m = mStart; m < mEnd; m++) {
            for (int n = nStart; n < nEnd; n++) {
                float acc = twoDimRead(QK_t, m, n, N);
                for (int kk = k; kk < kEnd; kk++) {
                    acc += fourDimRead(Q, selectB, selectH, m, kk, H, M, K) *
                           fourDimRead(K_t, selectB, selectH, n, kk, H, N, K);
                }
                twoDimWrite(QK_t, m, n, N, acc);
            }
        }
    }

    return true;
}

bool Matmul2Dto4D(std::vector<float> &QK_t, std::vector<float> &V, std::vector<float> &O, std::vector<int> QK_tDims,
                  std::vector<int> VDims, int selectB, int selectH, int mStart, int mEnd, int nStart, int nEnd)
{
    if (QK_tDims.size() != 2 || VDims.size() != 4) {
        std::cerr << "Invalid dimensions for QK^t or V." << std::endl;
        return false;
    }
    if (QK_tDims[1] != VDims[2] ) {
        std::cerr << "Sequence length of QK^t must match sequence length of V." << std::endl;
        std::cerr << "QK^t Dims: " << QK_tDims[0] << " " << QK_tDims[1] << std::endl;
        std::cerr << "V Dims: " << VDims[0] << " " << VDims[1] << " " << VDims[2] << " " << VDims[3] << std::endl;
        return false;
    }

    int B = VDims[0];
    int H = VDims[1];
    int K = VDims[2];
    int N = VDims[3];
    int M = QK_tDims[0];

    const int K_TILE = 32;

    // Zero the output tile
    for (int m = mStart; m < mEnd; m++)
        for (int n = nStart; n < nEnd; n++) {
            float zero = 0.0f;
            fourDimWrite(O, selectB, selectH, m, n, H, M, N, zero);
        }

    // K-tiled accumulation
    for (int k = 0; k < K; k += K_TILE) {
        int kEnd = std::min(k + K_TILE, K);
        for (int m = mStart; m < mEnd; m++) {
            for (int n = nStart; n < nEnd; n++) {
                float acc = fourDimRead(O, selectB, selectH, m, n, H, M, N);
                for (int kk = k; kk < kEnd; kk++) {
                    acc += twoDimRead(QK_t, m, kk, K) *
                           fourDimRead(V, selectB, selectH, kk, n, H, K, N);
                }
                fourDimWrite(O, selectB, selectH, m, n, H, M, N, acc);
            }
        }
    }

    return true;
}

bool SoftMax2D(std::vector<float> &mat,std::vector<int> matDims, int mStart,int mEnd)
{
    if (matDims.size() != 2) {
        std::cerr << "Invalid dimensions for matrix." << std::endl;
        return false;
    }
    int M = matDims[0];
    int N = matDims[1];
    for (int m = mStart; m < mEnd; m++)
    {
        float sumExp = 0.0;
        for (int n = 0; n < N; n++){
            float res = exp(twoDimRead(mat, m, n, N));
            sumExp += res;
            twoDimWrite(mat, m, n, N, res);
        }
        for (int n = 0; n < N; n++)
        {
            float val = twoDimRead(mat, m, n, N);
            float res = val / sumExp;
            twoDimWrite(mat, m, n, N, res);
        }
    }

    return true;
}

torch::Tensor myNaiveAttention(torch::Tensor QTensor, torch::Tensor KTensor, torch::Tensor VTensor, torch::Tensor QK_tTensor,
                int B, int H, int N, int d){

    // Q, K, V are passed in with Shape: (B, H, N, d)
    //QK^t Intermediate Tensor has Shape (N, N)
    
    //Make O Tensor with Shape (B, H, N, d) 
    at::Tensor OTensor = at::zeros({B, H, N, d}, at::kFloat);

    //Format O, Q, K, and V tensors into 4D vectors
    std::vector<float> O = formatTensor(OTensor);
    std::vector<float> Q = formatTensor(QTensor);
    std::vector<float> K = formatTensor(KTensor);
    std::vector<float> V = formatTensor(VTensor);

    //Format QK_t Tensor into a 2D vector.
    std::vector<float> QK_t = formatTensor(QK_tTensor);
    
    /* Here is an example of how to read/write 0's to  Q (B, H, N, d) using the 4D accessors

        //loop over Batch Size
         for (int b = 0; b < B; b++) {

             //loop over Heads
             for (int h = 0; h < H; h++) {

                 //loop over Sequence Length
                 for (int i = 0; i < N; i++) {

                     //loop over Embedding Dimensionality
                     for (int j = 0; j < d; j++) {
                        float val = fourDimRead(Q, b, h, i, j, H, N, d);
                        val = 0.0;
                        fourDimWrite(Q, b, h, i, j, H, N, d, val);
                     }
                 }
             }
         }
    */

    /* Here is an example of how to read/write 0's to  QK_t (N, N) using the 2D accessors

           for (int i = 0; i < N; i++) {
	       for (int j = 0; j < N; j++) {
	           float val = twoDimRead(QK_t, i, j, N);
               val = 0.0;
	           twoDimWrite(QK_t, i, j, N, val);
             }
         }
    */
    
    int TILE = 64;

    for (int b = 0; b < B; b++)
    {
        for(int h = 0; h < H; h++)
        {
            // Compute QK^t and store in QK_t (tiled on m, n)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                for (int tn = 0; tn < N; tn += TILE)
                {
                    int nEnd = std::min(tn + TILE, N);
                    MatmulTranspose4Dto2D(Q, K, QK_t, {B, H, N, d}, {B, H, N, d}, b, h, tm, mEnd, tn, nEnd);
                }
            }

            // Apply Softmax to QK_t (tiled on m)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                SoftMax2D(QK_t, {N, N}, tm, mEnd);
            }

            // Compute O = QK^tV and store in O (tiled on m, n)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                for (int tn = 0; tn < d; tn += TILE)
                {
                    int nEnd = std::min(tn + TILE, d);
                    Matmul2Dto4D(QK_t, V, O, {N, N}, {B, H, N, d}, b, h, tm, mEnd, tn, nEnd);
                }
            }
        }
    }
    
    
    // DO NOT EDIT THIS RETURN STATEMENT //
    // It formats your C++ Vector O back into a Tensor of Shape (B, H, N, d) and returns it //
    return torch::from_blob(O.data(), {B, H, N, d}, torch::TensorOptions().dtype(torch::kFloat32)).clone();
}


// ---------------------------------------------------------- //
//     PART 2: BLOCKED MATRIX MULTIPLY AND UNFUSED SOFTMAX    //
// ---------------------------------------------------------- //

torch::Tensor myUnfusedAttentionBlocked(torch::Tensor QTensor, torch::Tensor KTensor, torch::Tensor VTensor, torch::Tensor QK_tTensor,
                int B, int H, int N, int d){
    
    // Q, K, V are passed in with Shape: (B, H, N, d)
    //QK^t Intermediate Tensor has Shape (N, N)

    //Make O Tensor with Shape (B, H, N, d) 
    at::Tensor OTensor = at::zeros({B, H, N, d}, at::kFloat);

    //Format O, Q, K, and V tensors into 4D vectors
    std::vector<float> O = formatTensor(OTensor);
    std::vector<float> Q = formatTensor(QTensor);
    std::vector<float> K = formatTensor(KTensor);
    std::vector<float> V = formatTensor(VTensor);

    //Format QK_t Tensor into a 2D vector.
    std::vector<float> QK_t = formatTensor(QK_tTensor);

    // -------- YOUR CODE HERE  -------- //
    int TILE = 64;

    for (int b = 0; b < B; b++)
    {
        for(int h = 0; h < H; h++)
        {
            // Compute QK^t and store in QK_t (tiled on m, n)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                for (int tn = 0; tn < N; tn += TILE)
                {
                    int nEnd = std::min(tn + TILE, N);
                    MatmulTranspose4Dto2D(Q, K, QK_t, {B, H, N, d}, {B, H, N, d}, b, h, tm, mEnd, tn, nEnd);
                }
            }

            // Apply Softmax to QK_t (tiled on m)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                SoftMax2D(QK_t, {N, N}, tm, mEnd);
            }

            // Compute O = QK^tV and store in O (tiled on m, n)
            for (int tm = 0; tm < N; tm += TILE)
            {
                int mEnd = std::min(tm + TILE, N);
                for (int tn = 0; tn < d; tn += TILE)
                {
                    int nEnd = std::min(tn + TILE, d);
                    Matmul2Dto4D(QK_t, V, O, {N, N}, {B, H, N, d}, b, h, tm, mEnd, tn, nEnd);
                }
            }
        }
    }

    // DO NOT EDIT THIS RETURN STATEMENT //
    // It formats your C++ Vector O back into a Tensor of Shape (B, H, N, d) and returns it //
    return torch::from_blob(O.data(), {B, H, N, d}, torch::TensorOptions().dtype(torch::kFloat32)).clone();
}


// ---------------------------------------------------------- //
//                 PART 3: FUSED ATTENTION     	              //
// ---------------------------------------------------------- //

bool MatmulTranspose4DtoRow(std::vector<float> &Q, std::vector<float> &K_t, std::vector<float> &QK_t,
                           std::vector<int> QDims, std::vector<int> K_tDims, int selectB, int selectH, int seletctM)
{
    if (QDims.size() != 4 || K_tDims.size() != 1 ) {
        std::cerr << "Invalid dimensions for Q, K^t." << std::endl;
        return false;
    }
    if (QDims[3] != K_tDims[0])
    {
        std::cerr << "Batch size, number of heads, and embedding dimensionality must match for Q and K^t." << std::endl;
        std::cerr << "Q Dims: " << QDims[0] << " " << QDims[1] << " " << QDims[2] << " " << QDims[3] << std::endl;
        std::cerr << "K^t Dims: " << K_tDims[0] << std::endl;
        return false;
    }
     
    int B = QDims[0];
    int H = QDims[1];
    int M = QDims[2];
    int N = QDims[2];
    int K = QDims[3];

    for (int n = 0; n < N; n++) {
        float acc = 0;
        for (int k = 0; k < K; k++)
        {
            acc += fourDimRead(Q, selectB, selectH, seletctM, k, H, M, K) * fourDimRead(K_t, selectB, selectH, n, k, H, M, K);
        }
        QK_t[n] = acc;
    }

    return true;
}

bool MatmulRowTo4D(std::vector<float> &QK_t, std::vector<float> &V, std::vector<float> &O, std::vector<int> QK_tDims,
                   std::vector<int> VDims, int selectB, int selectH, int selectM)
{
    if (QK_tDims.size() != 1 || VDims.size() != 4) {
        std::cerr << "Invalid dimensions for QK^t or V." << std::endl;
        return false;
    }
    if (QK_tDims[0] != VDims[2] ) {
        std::cerr << "Sequence length of QK^t must match sequence length of V." << std::endl;
        std::cerr << "QK^t Dims: " << QK_tDims[0] <<  std::endl;
        std::cerr << "V Dims: " << VDims[0] << " " << VDims[1] << " " << VDims[2] << " " << VDims[3] << std::endl;
        return false;
    }

    int B = VDims[0];
    int H = VDims[1];
    int K = VDims[2];
    int N = VDims[3];
    int M = QK_tDims[0];

    for (int n = 0; n < N; n++) {
        float acc = 0;
        for (int k = 0; k < K; k++) {
            int myZero  = 0;
            acc += QK_t[k] * fourDimRead(V, selectB, selectH, k, n, H, K, N);
        }
        fourDimWrite(O, selectB, selectH, selectM, n, H, M, N, acc);
    }

    return true;
}

bool SoftMaxRow(std::vector<float> &mat, int len)
{
    float sumExp = 0.0;
    for (int i = 0; i < len; i++)
    {
        mat[i] = exp(mat[i]);
        sumExp += mat[i];
    }
    for (int i = 0; i < len; i++)
    {
        mat[i] = mat[i] / sumExp;
    }

    return true;
}

torch::Tensor myFusedAttention(torch::Tensor QTensor, torch::Tensor KTensor, torch::Tensor VTensor, torch::Tensor temp,
                int B, int H, int N, int d){

    // Q, K, V are passed in with Shape: (B, H, N, d)

    //Make O Tensor with Shape (B, H, N, d)
    //and O Row Tensor with Shape (N)
    at::Tensor OTensor = at::zeros({B, H, N, d}, at::kFloat);
    at::Tensor ORowTensor = at::zeros({N}, at::kFloat);

    //Format Y, Q, K, and V tensors into 4D vectors
    std::vector<float> O = formatTensor(OTensor);
    std::vector<float> Q = formatTensor(QTensor);
    std::vector<float> K = formatTensor(KTensor);
    std::vector<float> V = formatTensor(VTensor);
    
    //Format ORow Tensor into a 1D vector
    // You can simply access this as ORow[i]
    std::vector<float> ORow = formatTensor(ORowTensor);


    // -------- YOUR CODE HERE  -------- //
    // We give you a template of the first three loops for your convenience
    //loop over batch
    #pragma omp parallel for collapse(3)
    for (int b = 0; b < B; b++){

        //loop over heads
        for (int h = 0; h < H; h++){
            for (int i = 0; i < N ; i++){

		// YRow is moved inside so each OpenMP thread gets a local copy.
                at::Tensor ORowTensor = temp.index({torch::indexing::Slice(omp_get_thread_num(), torch::indexing::None)});      
                std::vector<float> ORow = formatTensor(ORowTensor);
                // YOUR CODE HERE
                MatmulTranspose4DtoRow(Q, K, ORow, {B, H, N, d}, {d}, b, h, i);
                SoftMaxRow(ORow, N);
                MatmulRowTo4D(ORow, V, O, {N}, {B, H, N, d}, b, h, i);
            }
	}
    }
	    
	
    // DO NOT EDIT THIS RETURN STATEMENT //
    // It formats your C++ Vector O back into a Tensor of Shape (B, H, N, d) and returns it //
    return torch::from_blob(O.data(), {B, H, N, d}, torch::TensorOptions().dtype(torch::kFloat32)).clone();
}


// ---------------------------------------------------------- //
//                PART 4: FLASH ATTENTION 		      //
// ---------------------------------------------------------- //

bool LoadKjVj(std::vector<float> &K, std::vector<float> &V, std::vector<float> &Kj, std::vector<float> &Vj,
              std::vector<int> KDims, int j, int Bc, int idxB, int idxH)
{
    int H = KDims[1];
    int N = KDims[2];
    int d = KDims[3];
    for (int row = 0; row < Bc; row++) {
        int seqIdx = j + row;
        for (int col = 0; col < d; col++) {
            Kj[row * d + col] = fourDimRead(K, idxB, idxH, seqIdx, col, H, N, d);
            Vj[row * d + col] = fourDimRead(V, idxB, idxH, seqIdx, col, H, N, d);
        }
    }
    return true;
}

bool LoadQiOiLi(std::vector<float> &Q, std::vector<float> &O, std::vector<float> &l, std::vector<float> &Qi,
                std::vector<float> &Oi, std::vector<float> &li, std::vector<int> QDims, int i, int Br, int idxB,
                int idxH)
{
    int H = QDims[1];
    int N = QDims[2];
    int d = QDims[3];
    for (int row = 0; row < Br; row++) {
        int seqIdx = i + row;
        li[row] = l[seqIdx];
        for (int col = 0; col < d; col++) {
            Qi[row * d + col] = fourDimRead(Q, idxB, idxH, seqIdx, col, H, N, d);
            Oi[row * d + col] = fourDimRead(O, idxB, idxH, seqIdx, col, H, N, d);
        }
    }
    return true;
}

bool ComputeSij(std::vector<float> &Qi, std::vector<float> &Kj, std::vector<float> &Sij, std::vector<int> QiDims,
                std::vector<int> KjDims, std::vector<int> SijDims, int idxB, int idxH)
{
    int Br = QiDims[0];
    int d  = QiDims[1];
    int Bc = KjDims[0];
    for (int m = 0; m < Br; m++) {
        for (int n = 0; n < Bc; n++) {
            float acc = 0.0f;
            for (int k = 0; k < d; k++) {
                acc += Qi[m * d + k] * Kj[n * d + k];
            }
            Sij[m * Bc + n] = acc;
        }
    }
    return true;
}

bool ComputePij(std::vector<float> &Sij, std::vector<float> &Pij, std::vector<int> SijDims, std::vector<int> PijDims,
                int idxB, int idxH)
{
    int Br = SijDims[0];
    int Bc = SijDims[1];
    for (int m = 0; m < Br; m++) {
        for (int n = 0; n < Bc; n++) {
            Pij[m * Bc + n] = exp(Sij[m * Bc + n]);
        }
    }
    return true;
}

bool ComputeLij(std::vector<float> &lij, std::vector<float> &Pij, std::vector<int> lijDims, std::vector<int> PijDims,
                int idxB, int idxH)
{
    int Br = lijDims[0];
    int Bc = PijDims[1];
    for (int m = 0; m < Br; m++) {
        float sum = 0.0f;
        for (int n = 0; n < Bc; n++) {
            sum += Pij[m * Bc + n];
        }
        lij[m] = sum;
    }
    return true;
}

bool ComputeLnew(std::vector<float> &lij, std::vector<float> &li, std::vector<float> &lnew, std::vector<int> lijDims,
                 std::vector<int> liDims, std::vector<int> lnewDims, int idxB, int idxH)
{
    int Br = lijDims[0];
    for (int m = 0; m < Br; m++) {
        lnew[m] = li[m] + lij[m];
    }
    return true;
}

bool ComputeOi(std::vector<float> &Pij, std::vector<float> &Vj, std::vector<float> &li, std::vector<float> &Oi,
               std::vector<float> &lnew, std::vector<int> PijDims, std::vector<int> VjDims, std::vector<int> liDims,
               std::vector<int> OiDims, std::vector<int> lnewDims, int idxB, int idxH)
{
    int Br = PijDims[0];
    int Bc = PijDims[1];
    int d  = VjDims[1];
    for (int m = 0; m < Br; m++) {
        for (int n = 0; n < d; n++) {
            // P_ij @ V_j contribution
            float pv = 0.0f;
            for (int k = 0; k < Bc; k++) {
                pv += Pij[m * Bc + k] * Vj[k * d + n];
            }
            // O_i <- (l_i * O_i + P_ij @ V_j) / l_new
            Oi[m * d + n] = (li[m] * Oi[m * d + n] + pv) / lnew[m];
        }
    }
    return true;
}

bool WriteOiToO(std::vector<float> &Oi, std::vector<float> &O, std::vector<int> OiDims, std::vector<int> ODims, int i,
           int idxB, int idxH)
{
    int Br = OiDims[0];
    int d  = OiDims[1];
    int H  = ODims[1];
    int N  = ODims[2];
    for (int row = 0; row < Br; row++) {
        int seqIdx = i + row;
        for (int col = 0; col < d; col++) {
            float val = Oi[row * d + col];
            fourDimWrite(O, idxB, idxH, seqIdx, col, H, N, d, val);
        }
    }
    return true;
}

bool WriteLnewToL(std::vector<float> &lnew, std::vector<float> &l, std::vector<int> lnewDims, std::vector<int> lDims, int i,
             int idxB, int idxH)
{
    int Br = lnewDims[0];
    for (int row = 0; row < Br; row++) {
        l[i + row] = lnew[row];
    }
    return true;
}

bool ResetL(std::vector<float> &l, std::vector<int> lDims)
{
    int N = lDims[0];
    for (int i = 0; i < N; i++) {
        l[i] = 0.0f;
    }
    return true;
}

torch::Tensor myFlashAttention(torch::Tensor QTensor, torch::Tensor KTensor, torch::Tensor VTensor,
               torch::Tensor QiTensor, torch::Tensor KjTensor, torch::Tensor VjTensor,
               torch::Tensor SijTensor, torch::Tensor PijTensor, torch::Tensor PVTensor,
               torch::Tensor OiTensor, torch::Tensor LTensor,  torch::Tensor LiTensor, 
	       torch::Tensor LijTensor, torch::Tensor LnewTensor, int Bc, int Br,
                int B, int H, int N, int d) {
        
    // Q, K, V are passed in with Shape: (B, H, N, d)
    // Sij, Pij are passed in with Shape: (Br, Bc)
    // Kj, Vj are passed in with Shape: (Bc, d)
    // Qi, Oi, and PV  are passed in with Shape: (Br, d)
    // L in passed in with Shape: (N)
    // Li, Lij, and Lnew are passed in with shape (Br)

    //Make O Tensor with Shape (B, H, N, d)
    at::Tensor OTensor = at::zeros({B, H, N, d}, at::kFloat);
   
    //Format All Tensors into Vectors
    std::vector<float> O = formatTensor(OTensor);
    std::vector<float> Q = formatTensor(QTensor);
    std::vector<float> K = formatTensor(KTensor);
    std::vector<float> V = formatTensor(VTensor);
    std::vector<float> Sij = formatTensor(SijTensor);
    std::vector<float> Pij = formatTensor(PijTensor);
    std::vector<float> Kj = formatTensor(KjTensor);
    std::vector<float> Vj = formatTensor(VjTensor);
    std::vector<float> Qi = formatTensor(QiTensor);
    std::vector<float> Oi = formatTensor(OiTensor);
    std::vector<float> l = formatTensor(LTensor);
    std::vector<float> PV = formatTensor(PVTensor);
    std::vector<float> li = formatTensor(LiTensor);
    std::vector<float> lij = formatTensor(LijTensor);
    std::vector<float> lnew = formatTensor(LnewTensor);

    // -------- YOUR CODE HERE  -------- //
    for (int idxB = 0; idxB < B; idxB++)
    {
        for (int idxH = 0; idxH < H; idxH++)
        {
            ResetL(l, {N});
            for (int j = 0; j < N; j += Bc)
            {
                int jEnd = std::min(j + Bc, N);
                int actualBc = jEnd - j;
                LoadKjVj(K, V, Kj, Vj, {B, H, N, d}, j, actualBc, idxB, idxH);
                for (int i = 0; i < N; i += Br)
                {
                    int iEnd = std::min(i + Br, N);
                    int actualBr = iEnd - i;
                    LoadQiOiLi(Q, O, l, Qi, Oi, li, {B, H, N, d}, i, actualBr, idxB, idxH);
                    ComputeSij(Qi, Kj, Sij, {actualBr, d}, {actualBc, d}, {actualBr, actualBc}, idxB, idxH);
                    ComputePij(Sij, Pij, {actualBr, actualBc}, {actualBr, actualBc}, idxB, idxH);
                    ComputeLij(lij, Pij, {actualBr}, {actualBr, actualBc}, idxB, idxH);
                    ComputeLnew(lij, li, lnew, {actualBr}, {actualBr}, {actualBr}, idxB, idxH);
                    ComputeOi(Pij, Vj, li, Oi, lnew, {actualBr, actualBc}, {actualBc, d}, {actualBr}, {actualBr, d}, {actualBr}, idxB, idxH);
                    WriteOiToO(Oi, O, {actualBr, d}, {B, H, N, d}, i, idxB, idxH);
                    WriteLnewToL(lnew, l, {actualBr}, {N}, i, idxB, idxH);
                }
            }
        }
    }

    // DO NOT EDIT THIS RETURN STATEMENT //
    // It formats your C++ Vector O back into a Tensor of Shape (B, H, N, d) and returns it //
    return torch::from_blob(O.data(), {B, H, N, d}, torch::TensorOptions().dtype(torch::kFloat32)).clone();
}


/* DO NOT EDIT THESE BINDINGS */
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("myNaiveAttention", &myNaiveAttention, "Naive Attention");
  m.def("myUnfusedAttentionBlocked", &myUnfusedAttentionBlocked, " Blocked Unfused Attention");
  m.def("myFusedAttention", &myFusedAttention, "Fused Attention");
  m.def("myFlashAttention", &myFlashAttention, "Flash Attention");
  m.def("twoDimRead", &twoDimRead, "twoDimRead");
  m.def("fourDimRead", &fourDimRead, "fourDimRead");
}
