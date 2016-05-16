#include <iostream>
#include <vector> 
#include <fstream>
#include <cassert>
#include <random>

using std::vector; 

class HashFunction { 
    private:
        uint64_t first_coeff_; 
        uint64_t second_coeff_; 
        uint64_t table_size_; 
        uint64_t prime_modulo_; 

    public:
        static constexpr uint64_t kBigPrime = 214783647;
        uint64_t operator() (const int key) const; 
        const uint64_t size() const; 
        template <class RandomGenerator>
        static HashFunction MakeRandom(const uint64_t size,
                                       RandomGenerator& random_numbers_generator, 
                                       const uint64_t prime_modulo = kBigPrime); 
        HashFunction(const uint64_t first, const uint64_t second, const uint64_t size, 
                     const uint64_t modulo);
        HashFunction() {} 
}; 

template <class T, class HashFunctionClass>
class InternalHashStructure { 
    private:
        HashFunctionClass function_; 
        vector<T> table_; 
        vector<bool> occupied_entries_; 
        void MakeCorrectFieldsFromTable(const vector<vector<T>>& table); 
    public:
        InternalHashStructure() {}
        template <class RandomGenerator>
        void Initialize(const vector<T>& keys, 
                        RandomGenerator& random_numbers_generator); 
        bool operator()(const T key) const; 
}; 

template <class T, class HashFunctionClass>
class FixedSet { 
    public: 
        FixedSet(); 
        void Initialize(const vector<T>& keys); 
        bool Contains(const T key) const; 
    private: 
        HashFunctionClass first_level_function_; 
        vector<InternalHashStructure<T, HashFunctionClass>> extern_table_; 
        std::mt19937 random_numbers_generator_; 
        static bool IsSquaredLengthsSumLinear(const vector<vector<T>>& big_table); 
        static constexpr uint32_t kLinearCoefficient = 4; 
}; 

        
vector<int> ReadKeys(std::istream& input_stream); 
vector<int> ReadRequests(std::istream& input_stream); 
vector<bool> MakeAnswers(const FixedSet<int, HashFunction>& set, 
                         const vector<int>& requests);
void PrintAnswers(const vector<bool>& answers, std::ostream& ouput_stream); 
template <class T, class HashFunctionClass> 
vector<vector<T>> PutInBuckets(const vector<T>& keys, const HashFunctionClass function);  
template <class T> 
bool CheckNoCollisions(const vector<vector<T>>& table); 

int main() { 
    std::istream& input_stream = std::cin; 
    std::ostream& output_stream = std::cout; 
    auto keys = ReadKeys(input_stream); 
    FixedSet<int, HashFunction> set; 
    set.Initialize(keys); 
    const vector<int> requests = ReadRequests(input_stream); 
    vector<bool> answers = MakeAnswers(set, requests); 
    PrintAnswers(answers, output_stream); 
    return 0; 
}

uint64_t HashFunction::operator() (const int key) const { 
    int residue = key % prime_modulo_; 
    uint64_t positive_residue = (residue < 0) ? (residue + prime_modulo_) : residue; 
    return ((first_coeff_ * positive_residue + second_coeff_) % prime_modulo_) % table_size_; 
}

HashFunction::HashFunction(const uint64_t first, const uint64_t second, 
                           const uint64_t size, const uint64_t modulo) : 
        first_coeff_(first), second_coeff_(second), table_size_(size), 
        prime_modulo_(modulo) {
}

template <class RandomGenerator>
HashFunction HashFunction::MakeRandom(const uint64_t size,
                                      RandomGenerator& random_numbers_generator,
                                      const uint64_t prime_modulo) { 
    std::uniform_int_distribution<size_t> first_distribution(1, prime_modulo - 1); 
    uint64_t first = first_distribution(random_numbers_generator); 
    std::uniform_int_distribution<size_t> second_distribution(0, prime_modulo - 1); 
    uint64_t second = second_distribution(random_numbers_generator); 
    return HashFunction(first, second, size, prime_modulo); 
}
    
                                                        
const uint64_t HashFunction::size() const { 
    return table_size_; 
}

template <class T, class HashFunctionClass> template <class RandomGenerator>
void InternalHashStructure<T, HashFunctionClass>::Initialize(const vector<T>& keys,
                                         RandomGenerator& random_numbers_generator) {
    if (keys.empty()) { 
        return; 
    }
    uint64_t size = keys.size() * keys.size(); 
    vector<vector<T>> table; 
    HashFunctionClass function;
    do { 
        function = HashFunctionClass::template MakeRandom<RandomGenerator>(size, 
                                                                random_numbers_generator);
        table = PutInBuckets<T, HashFunctionClass>(keys, function); 
    } while (!CheckNoCollisions<T>(table)); 
    function_ = function; 
    MakeCorrectFieldsFromTable(table); 
}

template <class T, class HashFunctionClass> 
void InternalHashStructure<T, HashFunctionClass>::MakeCorrectFieldsFromTable(
                                                            const vector<vector<T>>& table) { 
    table_.assign(table.size(), T());
    occupied_entries_ = vector<bool>(table.size()); 
    for (int i = 0; i < table.size(); ++i) { 
        if (!table[i].empty()) { 
            occupied_entries_[i] = true; 
            table_[i] = table[i][0]; 
        }
    }
}

template <class T, class HashFunctionClass>
bool InternalHashStructure<T, HashFunctionClass>::operator() (const T key) const { 
    if (table_.empty()) { 
        return false; 
    }
    uint64_t int_hash_code = function_(key); 
    return occupied_entries_[int_hash_code] ? (table_[int_hash_code] == key) : false;
}

template <class T, class HashFunctionClass>
FixedSet<T, HashFunctionClass>::FixedSet() { 
    std::random_device random; 
    random_numbers_generator_ = std::mt19937 (random()); 
}

template <class T, class HashFunctionClass>
void FixedSet<T, HashFunctionClass>::Initialize (const vector<T>& keys) { 
    const uint32_t set_size = keys.size(); 
    vector<std::vector<T>> big_table; 
    HashFunctionClass function; 
    do { 
        function = HashFunctionClass::MakeRandom(set_size, random_numbers_generator_); 
        big_table = PutInBuckets(keys, function); 
    } while (!IsSquaredLengthsSumLinear(big_table));
    first_level_function_ = function; 
    extern_table_.assign(set_size, InternalHashStructure<T, HashFunctionClass>());
    for (int index = 0; index < set_size; ++index) { 
            extern_table_[index].Initialize(big_table[index], random_numbers_generator_); 
    }
}

template <class T, class HashFunctionClass>
bool FixedSet<T, HashFunctionClass>::Contains(const T key) const { 
    size_t ext_hash_code = first_level_function_(key); 
    return extern_table_[ext_hash_code](key); 
}

template <class T, class HashFunctionClass>
bool FixedSet<T, HashFunctionClass>::IsSquaredLengthsSumLinear(
                                                  const vector<std::vector<T>>& big_table) { 
    auto size = big_table.size(); 
    auto up_border = kLinearCoefficient * size; 
    uint64_t sum_of_squares = 0; 
    for (auto& entry : big_table) { 
            sum_of_squares += entry.size() * entry.size(); 
    }
    return (sum_of_squares < up_border); 
}

template <class T, class HashFunctionClass>
vector<vector<T>> PutInBuckets(const vector<T>& keys, const HashFunctionClass function) { 
    vector<vector<T>> big_table(function.size()); 
    for (const auto key : keys) { 
        uint64_t hash_code = function(key); 
        big_table[hash_code].push_back(key); 
    }
    return big_table; 
}

template <class T> 
bool CheckNoCollisions(const vector<vector<T>>& table) { 
    for (const auto& intern_table : table) { 
        if (intern_table.size() > 1) { 
            return false; 
        }
    }
    return true; 
}

vector<int> ReadKeys(std::istream& input_stream) { 
    size_t num_of_keys; 
    input_stream >> num_of_keys; 
    vector<int> keys(num_of_keys); 
    for (auto& key : keys) { 
        input_stream >> key; 
    }
    return keys; 
}

vector<int> ReadRequests(std::istream& input_stream) { 
    return ReadKeys(input_stream);
}

vector<bool> MakeAnswers(const FixedSet<int, HashFunction>& set, 
                         const vector<int>& requests) { 
    vector<bool> answers; 
    answers.reserve(requests.size()); 
    for (const auto request : requests) { 
        answers.push_back(set.Contains(request)); 
    }
    return answers; 
}

void PrintAnswers(const vector<bool>& answers, std::ostream& output_stream) { 
    for (auto answer : answers) { 
        if (answer) {
            output_stream << "Yes\n"; 
        } else { 
            output_stream << "No\n"; 
        }
    }
}

