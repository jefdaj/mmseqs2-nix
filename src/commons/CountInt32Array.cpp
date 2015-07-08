#include "CountInt32Array.h"
#include <iostream>

CountInt32Array::CountInt32Array(size_t maxElement, size_t initBinSize) {
    // find nearest upper power of 2^(x)
    size_t size = pow(2, ceil(log(maxElement)/log(2)));
    size = size  >> (MASK_0_5_BIT + 3); // space needed in bit array
    size = std::max(size, (size_t) 1); // minimal size of 1;
    duplicateBitArraySize = size;
    duplicateBitArray = new unsigned char[size];
    memset(duplicateBitArray, 0, duplicateBitArraySize * sizeof(unsigned char));
    // init data for bins
    binCount = MASK_0_5 + 1;
    // find nearest upper power of 2^(x)
    initBinSize = pow(2, ceil(log(initBinSize)/log(2)));
    binSize = initBinSize;
    // pointer to start points of the bins
    bins = new unsigned int *[binCount];
    binDataFrame = new unsigned int[binCount * binSize];
    tmpElementBuffer = new unsigned int[binSize];
    // init sub bin data
    subBinCount = (MASK_6_11 >> MASK_0_5_BIT) + 1;
    subBins = new unsigned int *[subBinCount];
    subBinSize = initBinSize; //TODO
    subBinDataFrame = new unsigned int[subBinCount * subBinSize];
    // find memory needed for lookup
    unsigned int maxElementHighestBit = highest_bit_set((size_t)pow(2, ceil(log(maxElement)/log(2))));
    unsigned int exponent = (maxElementHighestBit <= (MASK_0_5_BIT + MASK_6_11_BIT))
                            ? 1 : maxElementHighestBit - (MASK_0_5_BIT + MASK_6_11_BIT);
    size_t lookupSize = pow(2, exponent);
    lookup = new unsigned char[lookupSize];
}

unsigned int CountInt32Array::highest_bit_set(size_t num)
{
    unsigned int count = 0;
    while (num >> 1 != 0) {
        count++;
        num = num >> 1;
    }
    return(count);
}

CountInt32Array::~CountInt32Array(){
    delete [] duplicateBitArray;
    delete [] bins;
    delete [] binDataFrame;
    delete [] subBins;
    delete [] subBinDataFrame;
    delete [] lookup;
    delete [] tmpElementBuffer;
}

size_t CountInt32Array::countElements(unsigned int *inputArray, const size_t N,  CounterResult * output) {
    newStart:
    setupBinPointer(bins, binCount, binDataFrame, binSize);
    hashElements(inputArray, N, this->bins);
    if(checkForOverflowAndResizeArray(bins, binCount, binSize) == true) // overflowed occurred
        goto newStart;
    return findDuplicates(this->bins, this->binCount, this->subBins, this->subBinCount, output);
}

size_t CountInt32Array::findDuplicates(unsigned int **bins, unsigned int binCount,
                                       unsigned int **subBins, unsigned int subBinCount,
                                       CounterResult * output) {
    size_t doubleElementCount = 0;
    const unsigned int * bin_ref_pointer = binDataFrame;
    for (size_t bin = 0; bin < binCount; bin++) {
        const unsigned int *binStartPos = (bin_ref_pointer + bin * binSize);
        const size_t currBinSize = (bins[bin] - binStartPos);
//        size_t elementCount = 0;
//        // find duplicates
//        for (size_t n = 0; n < currBinSize; n++) {
//            const unsigned int element = binStartPos[n];
//            // turn off last four bits (they contain the diagonal)
//            const unsigned int hashBinElement = (element & 0x0FFFFFFF) >> (MASK_0_5_BIT);
//            const unsigned int byteArrayPos = (hashBinElement >> 3); // equal to  hashBinElement / 8
//            const unsigned char bitPosMask = 1 << (hashBinElement & 7);  // 7 = 00000111
//            // check if duplicate element was found before
//            tmpElementBuffer[elementCount] = element;
//            elementCount +=  (duplicateBitArray[byteArrayPos] & bitPosMask ) ? 1 : 0;
//            // set element corresponding bit in byte
//            duplicateBitArray[byteArrayPos] |= bitPosMask;
//        }
//        // clean memory faster if current bin size is smaller duplicateBitArraySize
//        if(currBinSize < duplicateBitArraySize / 16){
//            for (size_t n = 0; n < currBinSize; n++) {
//                const unsigned int element = (binStartPos[n] & 0x0FFFFFFF);
//                const unsigned int byteArrayPos = element >> (MASK_0_5_BIT + 3);
//                duplicateBitArray[byteArrayPos] = 0;
//            }
//        }else{
//            memset(duplicateBitArray, 0, duplicateBitArraySize * sizeof(unsigned char));
//        }
        setupBinPointer(subBins, subBinCount, subBinDataFrame, subBinSize);
        doubleElementCount += countDuplicates(subBins, subBinCount, binStartPos, currBinSize, output + doubleElementCount);
    }

    return doubleElementCount;
}

bool CountInt32Array::checkForOverflowAndResizeArray(unsigned int **bins,
        const unsigned int binCount,
        const size_t binSize) {
    const unsigned int * bin_ref_pointer = binDataFrame;
    unsigned int * lastPosition = (binDataFrame + binCount * binSize) - 1;
    for (size_t bin = 0; bin < binCount; bin++) {
        const unsigned int *binStartPos = (bin_ref_pointer + bin * binSize);
        const size_t n = (bins[bin] - binStartPos);
        // if one bin has more elements than BIN_SIZE
        // or the current bin pointer is at the end of the binDataFrame
        // reallocate new memory
        if( n > binSize || bins[bin] >= lastPosition) {
            // overflow detected
            // find nearest upper power of 2^(x)
            std::cout << "Found overlow" << std::endl;
            this->binSize = pow(2, ceil(log(binSize + 1)/log(2)));
            reallocBinMemory(binCount, this->binSize);
            return true;
        }
    }
    return false;
}

void CountInt32Array::reallocBinMemory(const unsigned int binCount, const size_t binSize) {
    delete [] binDataFrame;
    delete [] tmpElementBuffer;
    binDataFrame     = new unsigned int[binCount * binSize];
    tmpElementBuffer = new unsigned int[binSize];
}

size_t CountInt32Array::countDuplicates(unsigned int ** subHashBin, unsigned int subBinCount,
                                        const unsigned int *inputArray, const unsigned int N, CounterResult * output) {
    // hash element again
    unsigned int *lastPosition = (subBinDataFrame + subBinCount * subBinSize) - 1;
    for (size_t n = 0; n < N; n++) {
        const unsigned int element = inputArray[n];
        const unsigned int bin_id = (element & MASK_6_11) >> (MASK_0_5_BIT);
        *subHashBin[bin_id] = element;
        // do not write over boundary of the data frame
        subHashBin[bin_id] += (subHashBin[bin_id] >= lastPosition) ? 0 : 1;
    }
    // count dup.
    size_t pos = 0;
    const unsigned int * subBin_ref_pointer = subBinDataFrame;
    for (size_t subBin = 0; subBin < subBinCount; subBin++) {
        const unsigned int *binStartPos = (subBin_ref_pointer + subBin * subBinSize);
        const size_t currBinSize = (subHashBin[subBin] - binStartPos);
        for (size_t n = 0; n < currBinSize; n++) {
            const unsigned int id            = (binStartPos[n] & 0x0FFFFFFF) >> (MASK_6_11_BIT + MASK_0_5_BIT);
            const unsigned char currDiagonal = (binStartPos[n] >> 28);
            const unsigned char score    = (lookup[id] & 0x0F); //00001111
            const unsigned char prevDiagonal = (lookup[id] & 0xF0) >> 4; //11110000 -> 00001010
            const unsigned char newScore = score + (prevDiagonal == currDiagonal && score < 16) ? 1 : 0;
            lookup[id] = newScore;
            lookup[id] |= (prevDiagonal >> 4); // set diagonal
        }
        // extract results and set memory to
        for (size_t n = 0; n < currBinSize; n++) {
            const unsigned int id = (binStartPos[n] & 0x0FFFFFFF);
            const unsigned int element = id >> (MASK_6_11_BIT + MASK_0_5_BIT);
            output[pos].id    = id;
            output[pos].count = lookup[element];
            pos += (lookup[element] != 0) ? 1 : 0; //TODO avoid memory shit
            lookup[element] = 0;
        }
    }
    return pos;
}

void CountInt32Array::setupBinPointer(unsigned int **bins, const unsigned int binCount,
        unsigned int *binDataFrame, const size_t binSize)
{
    // Example binCount = 3
    // bin start             |-----------------------|-----------------------| bin end
    //    segments[bin_step][0]
    //                            segments[bin_step][1]
    //                                                    segments[bin_step][2]
    size_t curr_pos = 0;
    for(size_t bin = 0; bin < binCount; bin++){
        bins[bin] = binDataFrame + curr_pos;
        curr_pos += binSize;
    }
}

void CountInt32Array::hashElements(unsigned int const *inputArray, size_t N, unsigned int **hashBins)
{
    unsigned int * lastPosition = (binDataFrame + binCount * binSize) - 1;
    for(size_t n = 0; n < N; n++) {
        const unsigned int element = inputArray[n];
        const unsigned int bin_id = (element & MASK_0_5);
        *hashBins[bin_id] = element;
        // do not write over boundary of the data frame
        hashBins[bin_id] += (hashBins[bin_id] >= lastPosition) ? 0 : 1;
    }
}