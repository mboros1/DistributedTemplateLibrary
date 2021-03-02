//
// Created by martin on 3/1/21.
//

#ifndef DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
#define DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H

namespace dtl {
#include <sys/mman.h>

    const size_t INITIAL_VEC_CAPACITY = 16;

    template<typename T>
    class DistributedVector {
    private:
        T* create_shared_memory(size_t size) {
            int protection = PROT_READ | PROT_WRITE;
            int visibility = MAP_SHARED | MAP_ANONYMOUS;

            // The remaining parameters to `mmap()` are not important for this use case,
            // but the manpage for `mmap` explains their purpose.
            void* result = mmap(NULL, size * sizeof(T), protection, visibility, -1, 0);
            if (result == MAP_FAILED){
                std::cerr << "Mapping failed!\n";
            }
            return (T*)result;
        }

        void resize_shared_memory(T* addr, size_t size, size_t new_size){
            m_begin = (T*)mremap(addr, size * sizeof(T), new_size * sizeof(T), MREMAP_MAYMOVE);
            if (m_begin == MAP_FAILED) {
                std::cerr << "Mapping failed!\n";
            }
            m_end = m_begin + size;
            m_end_cap = m_begin + new_size;
        }

    public:
        DistributedVector() {
            m_begin = create_shared_memory(INITIAL_VEC_CAPACITY);
            m_end_cap = m_begin + INITIAL_VEC_CAPACITY;
            m_end = m_begin;
        }

        size_t size() {
            return m_end - m_begin;
        }

        size_t capacity() {
            return m_end_cap - m_begin;
        }

        T* data(){
            return m_begin;
        }

        T at(size_t index){
            if (index < (m_end - m_begin)) {
                return *(m_begin+index);
            } else {
                return -1;
            }
        }

        T push_back(T ele){
            if (m_end == m_end_cap){
                resize_shared_memory(m_begin,size(), 2*capacity());
            }
            *m_end = ele;
            m_end++;
            return *m_end;
        }

    private:
        T* m_begin;
        T* m_end;
        T* m_end_cap;

    };
}

#endif //DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
