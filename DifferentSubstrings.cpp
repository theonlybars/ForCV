#include <iostream>
#include <vector> 
#include <cassert>
#include <string>
#include <algorithm>

using std::cin; 
using std::cout; 
using std::vector; 



class LargestCommonPrefix { 
    private: 
        std::string string_; 
        vector<int> lcp_; 
    public: 
        LargestCommonPrefix() = default; 
        ~LargestCommonPrefix() = default; 
        LargestCommonPrefix(const LargestCommonPrefix&) = default; 
        void Initialize(const std::string& string, const vector<int>& suffix_array); 
        int operator[](const size_t index); 
}; 

class SuffixArray { 
    private: 
        std::string string_; 
        vector<int> array_; 
        static vector<int> MakeSAFromIntegers(const vector<int>& string);  
        static vector<int> MakeTripplesString(const vector<int>& string);
        static vector<int> MakeSAFromSmallArray(const vector<int>& small_array,
                                                const vector<int>& string);
        static vector<int> Merge(const vector<int>& first, const vector<int>& second,
                                 const vector<int>& third);
        static vector<int> MakeColorsFromTuples(const vector<vector<int>>& tuples);
        static vector<int> MakeCorrectFromSmall(const vector<int>& small_array); 
        template<int tuple_length> static void FillSymbolColorTuplesArray(
                        vector<vector<int>>& suffixes, vector<int>& suffix_indexes,
                        const int modulo_first, const int modulo_second, 
                        const vector<int>& coloring, const vector<int>& string);
        static constexpr size_t kSmallSize = 20; 
    public: 
        SuffixArray() = default; 
        ~SuffixArray() = default; 
        SuffixArray(const SuffixArray& ) = default; 
        void Initialize(const std::string& string); 
        vector<int> Vector() const; 
}; 

struct Compare { 
    bool operator() (const vector<int>& first, const vector<int>& second) { 
        for (int i = 0; (i < first.size()) && (i < second.size()); ++i) { 
            if (first[i] < second[i]) { 
                return true; 
            } else if (second[i] < first[i]) { 
                return false; 
            }
        }
        if (first.size() <= second.size()) { 
            return true; 
        }  
        return false; 
    }
}; 

vector<int> ModifyToIntegers(const std::string& string); 
vector<int> RadixSort(const vector<vector<int>>& tuples);
vector<int> CountSort(const vector<int>& array); 
vector<int> PermutationMultiply(const vector<int>& left, const vector<int>& right);
vector<int> Inverse(const vector<int> permutation); 
vector<vector<int>> MakeSuffixesVector(const vector<int>& string); 
void PushTripples(vector<vector<int>>& tripples_array, const vector<int>& string, 
                  const int modulo); 
bool MajorNegative(int first, int second, int third); 

int main() { 
    SuffixArray suffix_array; 
    std::string input_string; 
    cin >> input_string; 
    suffix_array.Initialize(input_string); 
    LargestCommonPrefix lcp; 
    vector<int> suffixes_order = suffix_array.Vector(); 
    lcp.Initialize(input_string, suffixes_order); 
    long long substrings_number = 0; 
    int lcp_length = 0; 
    int length = input_string.size(); 
    int index = 0; 
    for (int suffix_index : suffixes_order) { 
        substrings_number += length - suffix_index - lcp_length; 
        if (index < length - 1) { 
            lcp_length = lcp[index]; 
        }
        ++index; 
    }
    cout << substrings_number; 
    return 0; 
}

void SuffixArray::Initialize(const std::string& string) { 
    string_ = string; 
    vector<int> integers_string = ModifyToIntegers(string); 
    array_ = MakeSAFromIntegers(integers_string); 
}

vector<int> SuffixArray::MakeSAFromIntegers(const vector<int>& string) { 
    int size = string.size(); 
    if (size < kSmallSize) { 
        vector<vector<int>> suffixes = MakeSuffixesVector(string); 
        std::sort(suffixes.begin(), suffixes.end(), Compare()); 
        vector<int> suffix_array(size); 
        for (int i = 0; i < size; ++i) { 
            suffix_array[i] = size - suffixes[i].size(); 
        }
        return suffix_array; 
    } else { 
        for (const auto number : string) { 
            assert(number > 0); 
        }
        vector<int> modified_string = string; 
        modified_string.push_back(0); 
        vector<int> tripples_string = MakeTripplesString(modified_string); 
        vector<int> small_suffix_array = MakeSAFromIntegers(tripples_string); 
        vector<int> modified_suffix_array = MakeSAFromSmallArray(small_suffix_array, 
                                                                 modified_string); 
        vector<int> suffix_array(modified_suffix_array.size() - 1); 
        std::copy(modified_suffix_array.begin() + 1, 
                  modified_suffix_array.end(), suffix_array.begin()); 
        return suffix_array; 
    }
}

vector<int> ModifyToIntegers(const std::string& string) { 
    vector<int> integers_string(string.size()); 
    int a_symbol = static_cast<int>('a'); 
    int z_symbol = static_cast<int>('z'); 
    for (int i = 0; i < string.size(); ++i) { 
        int symbol = static_cast<int>(string[i]); 
        assert(symbol >= a_symbol); 
        assert(symbol <= z_symbol); 
        integers_string[i] = static_cast<int>(string[i] - 'a') + 1; 
    }
    return integers_string; 
}
 
vector<int> SuffixArray::MakeTripplesString(const vector<int>& string) { 
    int length = string.size(); 
    vector<vector<int>> tripples_array; 
    tripples_array.reserve(2 * length / 3); 
    PushTripples(tripples_array, string, 0); 
    PushTripples(tripples_array, string, 1); 
    return MakeColorsFromTuples(tripples_array); 
}

vector<int> SuffixArray::MakeSAFromSmallArray(const vector<int>& small_array, 
                                              const vector<int>& string) { 
    int size = string.size(); 
    vector<int> zero_one_suffix_array = MakeCorrectFromSmall(small_array); 
    vector<int> known_coloring(size, -1); 
    for (int i = 0; i < zero_one_suffix_array.size(); ++i) { 
        known_coloring[zero_one_suffix_array[i]] = i + 1; 
    }

    vector<vector<int>> modulo_02_suffixes; 
    vector<int> modulo_02_suffixes_indexes; 
    FillSymbolColorTuplesArray<2>(modulo_02_suffixes, modulo_02_suffixes_indexes,
                                  0, 2, known_coloring, string); 

    vector<vector<int>> modulo_12_suffixes; 
    vector<int> modulo_12_suffixes_indexes; 
    FillSymbolColorTuplesArray<3>(modulo_12_suffixes, modulo_12_suffixes_indexes,
                                  1, 2, known_coloring, string); 

    vector<int> permutation_first = RadixSort(modulo_02_suffixes); 
    vector<int> zero_two_suffix_array(modulo_02_suffixes_indexes.size()); 
    for (int i = 0; i < zero_two_suffix_array.size(); ++i) { 
        zero_two_suffix_array[permutation_first[i]] = modulo_02_suffixes_indexes[i]; 
    }

    vector<int> permutation_second = RadixSort(modulo_12_suffixes); 
    vector<int> one_two_suffix_array(modulo_12_suffixes_indexes.size()); 
    for (int i = 0; i < one_two_suffix_array.size(); ++i) { 
        one_two_suffix_array[permutation_second[i]] = modulo_12_suffixes_indexes[i]; 
    }
    return Merge(zero_one_suffix_array, zero_two_suffix_array, one_two_suffix_array); 
}

vector<int> SuffixArray::MakeCorrectFromSmall(const vector<int>& small_array) { 
    int size = small_array.size(); 
    int delimiter = (size % 2 == 0) ? size / 2 : size / 2 + 1; 
    vector<int> correct_array(size); 
    for (int i = 0; i < size; ++i) { 
        correct_array[i] = (small_array[i] < delimiter) ?
                           (3 * small_array[i]) : (3 * (small_array[i] - delimiter) + 1); 
    }
    return correct_array; 
}

vector<int> SuffixArray::Merge(const vector<int>& first, const vector<int>& second,
                               const vector<int>& third) { 
    int doubled_merged_size = (first.size() + second.size() + third.size());  
    assert(doubled_merged_size % 2 == 0); 
    int merged_size = doubled_merged_size / 2; 
    vector<int> merged_array(merged_size); 
    int first_index = 0, second_index = 0, third_index = 0; 
    for (int index = 0; index < merged_size; ++index) { 
        int first_element = (first_index < first.size()) ? first[first_index] : -1; 
        int second_element = (second_index < second.size()) ? second[second_index] : -1; 
        int third_element  = (third_index < third.size()) ? third[third_index] : -1; 
        if (MajorNegative(first_element, second_element, third_element)) { 
            assert(false); 
        }
        if (first_element == second_element) { 
            merged_array[index] = first_element; 
            ++first_index, ++second_index; 
        } else if (second_element == third_element) { 
            merged_array[index] = second_element; 
            ++second_index, ++third_index; 
        } else if (first_element == third_element) { 
            merged_array[index] = first_element; 
            ++first_index, ++third_index; 
        } else { 
            assert(false); 
        }
    }        
    return merged_array; 
}

vector<int> SuffixArray::MakeColorsFromTuples(const vector<vector<int>>& tuples) { 
    vector<int> result_permutation = RadixSort(tuples); 
    vector<int> inverse_permutation = Inverse(result_permutation); 
    int size = tuples.size(); 
    vector<int> colors(size); 
    int current_color = 1; 
    colors[inverse_permutation[0]] = 1; 
    for (int index = 1; index < size; ++index) { 
        if (tuples[inverse_permutation[index]] != tuples[inverse_permutation[index - 1]]) { 
            ++current_color; 
        }
        colors[inverse_permutation[index]] = current_color; 
    }
    return colors; 
}

void LargestCommonPrefix::Initialize(const std::string& string, const vector<int>& suffix_array) { 
    string_ = string; 
    vector<int> inverse_suffix_array = Inverse(suffix_array); 
    int length = string.size(); 
    lcp_.assign(length - 1, -1); 
    int lcp = 0; 
    for (int index = 0; index < length; ++index) { 
        int order_number = inverse_suffix_array[index]; 
        if (order_number == length - 1) { 
            continue; 
        } 
        int next_suffix_index = suffix_array[order_number + 1]; 
        lcp = (lcp == 0) ? 0 : lcp - 1; 
        int lcp_addition = 0; 
        while (string[index + lcp + lcp_addition] == 
               string[next_suffix_index + lcp + lcp_addition]) { 
            ++lcp_addition; 
            if ((index + lcp + lcp_addition == length) || 
                (next_suffix_index + lcp + lcp_addition == length)) { 
                break; 
            }
        }
        lcp += lcp_addition; 
        lcp_[order_number] = lcp; 
    }
}

int LargestCommonPrefix::operator[](const size_t index) { 
    assert(index < lcp_.size()); 
    return lcp_[index]; 
}

vector<int> RadixSort(const vector<vector<int>>& tuples) { 
    int length; 
    if (tuples.empty()) { 
        return vector<int>(); 
    } else { 
        length = tuples[0].size(); 
    }
    for (const auto& tuple : tuples) { 
        if (tuple.size() != length) { 
            cout << "Wrong input in RadixSort" << std::endl; 
            assert(false); 
        }
    }
    int size = tuples.size(); 
    vector<int> result_permutation(size); 
    for (int i = 0; i < size; ++i) { 
        result_permutation[i] = i; 
    }
    for (int coordinate = length - 1; coordinate >= 0; --coordinate) { 
        vector<int> permutation(size); 
        vector<int> projection(size); 
        for (int index = 0; index < size; ++index) {
            projection[result_permutation[index]] = tuples[index][coordinate]; 
        }
        permutation = CountSort(projection); 
        result_permutation = PermutationMultiply(permutation, result_permutation); 
    }
    return result_permutation; 
}

vector<int> CountSort(const vector<int>& array) { 
    int maximum = 0; 
    for (const auto number : array) { 
        assert(number >= 0); 
        if (number > maximum) { 
            maximum = number; 
        }
    }
    vector<int> boxes_sizes(maximum + 1, 0); 
    for (const auto number : array) { 
        ++boxes_sizes[number]; 
    }
    vector<int> positions(maximum + 1, 0); 
    int start_indent = 0; 
    for (int i = 0; i < maximum + 1; ++i) { 
        positions[i] = start_indent; 
        start_indent += boxes_sizes[i]; 
    }
    vector<int> permutation(array.size()); 
    for (int i = 0; i < array.size(); ++i) { 
        permutation[i] = positions[array[i]]++; 
    }
    return permutation; 
}

vector<int> PermutationMultiply(const vector<int>& left, const vector<int>& right) { 
    vector<int> result_permutation(right.size()); 
    for (int i = 0; i < result_permutation.size(); ++i) { 
        result_permutation[i] = left[right[i]]; 
    }
    return result_permutation; 
}

vector<int> Inverse(const vector<int> permutation) { 
    if (permutation.empty()) { 
        return vector<int>(); 
    } 
    vector<int> result_permutation(permutation.size()); 
    for (int i = 0; i < permutation.size(); ++i) { 
        result_permutation[permutation[i]] = i; 
    }
    return result_permutation; 
}


vector<vector<int>> MakeSuffixesVector(const vector<int>& string) { 
    vector<vector<int>> suffixes; 
    for (int suffix_index = 0; suffix_index < string.size(); ++suffix_index) { 
        int new_suffix_length = string.size() - suffix_index; 
        suffixes.push_back(vector<int>(new_suffix_length)); 
        std::copy<>(string.begin() + suffix_index, string.end(), suffixes.back().begin()); 
    }
    return suffixes; 
}

void PushTripples(vector<vector<int>>& tripples_array, 
                  const vector<int>& string, const int modulo) { 
    int size = string.size(); 
    for (int tripple_begin = modulo; tripple_begin < size; tripple_begin += 3) { 
        int first_symbol = string[tripple_begin]; 
        int second_symbol = (tripple_begin + 1 < size) ? string[tripple_begin + 1] : 0; 
        int third_symbol = (tripple_begin + 2 < size) ? string[tripple_begin + 2] : 0; 
        auto init_list = {first_symbol, second_symbol, third_symbol}; 
        tripples_array.emplace_back(init_list); 
    }
}

template <int tuple_length>
void SuffixArray::FillSymbolColorTuplesArray(
                vector<vector<int>>& suffixes, vector<int>& suffix_indexes,
                const int modulo_first, const int modulo_second, 
                const vector<int>& coloring, const vector<int>& string) { 
    int size = string.size(); 
    for (int index = 0; index < size; ++index) { 
        int modulo = index % 3; 
        if ((modulo != modulo_first) && (modulo != modulo_second)) { 
            continue; 
        }
        vector<int> new_tuple(tuple_length); 
        new_tuple[0] = string[index]; 
        for (int j = 1; j + 1 < tuple_length; ++j) { 
            new_tuple[j] = (index + j < size) ? string[index + j] : 0; 
        }
        new_tuple[tuple_length - 1] = (index + tuple_length - 1 < size) ?
                                      coloring[index + tuple_length - 1] : 0; 
        assert(new_tuple[tuple_length] != -1); 
        suffixes.push_back(new_tuple); 
        suffix_indexes.push_back(index); 
    }
}

bool MajorNegative(int first, int second, int third) { 
    int negatives = 0; 
    if (first < 0) { 
        ++negatives; 
    } 
    if (second < 0) { 
        ++negatives; 
    } 
    if (third < 0) { 
        ++negatives; 
    }
    return (negatives > 1); 
}

vector<int> SuffixArray::Vector() const { 
    return array_; 
}
