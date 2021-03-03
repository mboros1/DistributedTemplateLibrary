//
// Created by martin on 3/1/21.
//

#ifndef DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
#define DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H

namespace dtl {
#include <sys/mman.h>
#include <unistd.h>

    template<typename T>
    class DistributedVector {
    private:
        ///
        /// \param size size_t size of shared memory; must be page aligned
        /// \return pointer to allocated shared memory
        T* create_shared_memory(size_t size) {
            int protection = PROT_READ | PROT_WRITE;
            int visibility = MAP_SHARED;

            m_shm_fd = memfd_create("shm", 0);
            ftruncate(m_shm_fd, size * sizeof(T));
            void* result = mmap(nullptr, size * sizeof(T), protection, visibility, m_shm_fd, 0);
            if (result == MAP_FAILED){
                throw std::runtime_error("Memory mapping failed");
            }
            file_size = lseek(m_shm_fd, 0, SEEK_END);
            return (T*)result;
        }

        ///
        /// \param size size_t current size of shared memory
        /// \param new_size size_t desired size of new memory
        void resize_shared_memory(size_t size, size_t new_size) {
            m_begin = (T*)mremap(m_begin, size*sizeof(T), new_size*sizeof(T), MREMAP_MAYMOVE);
            if (m_begin == MAP_FAILED){
                throw std::runtime_error("Memory remapping failed");
            }
            ftruncate(m_shm_fd, new_size * sizeof(T));
            m_end = m_begin + size;
            m_end_cap = m_begin + new_size;
        }

        void check_type_invariants(){
            if (std::is_pointer<T>::value){
                std::cout << "WARNING: pointers to heap memory will cause shared memory corruption\n";
            }
            if (!std::is_copy_assignable_v<T> && !std::is_copy_constructible_v<T>){
                throw std::invalid_argument("Template type must be copyable");
            }
        }

    public:
        DistributedVector() {
            check_type_invariants();
            m_begin = create_shared_memory(m_page_aligned_vec_size);
            m_end_cap = m_begin + m_page_aligned_vec_size;
            m_end = m_begin;
        }

        DistributedVector(size_t size){
            check_type_invariants();
            size_t alloc_size = m_page_aligned_vec_size;
            while(alloc_size < 2*size){
                alloc_size *= 2;
            }
            m_begin = create_shared_memory(alloc_size);
            m_end_cap = m_begin + alloc_size;
            m_end = m_begin + size;
        }

        ~DistributedVector(){
            munmap(m_begin, m_end_cap - m_begin);
        }

        ///
        /// \return size_t size of array in use
        size_t size() {
            return m_end - m_begin;
        }

        ///
        /// \return size_t capacity of allocated shared memory
        size_t capacity() {
            return m_end_cap - m_begin;
        }

        ///
        /// \return Pointer to underlying shared memory array
        T* data(){
            return m_begin;
        }

        ///
        /// \param index
        /// \return
        T& at(size_t index){
            if (index < (m_end - m_begin)) {
                return *(m_begin+index);
            } else {
                throw std::out_of_range("Array bound exception");
            }
        }

        ///
        /// \param ele element to be added to the vector; ele is copied
        /// \return index where ele was added
        size_t push_back(const T& ele){
            if (m_end == m_end_cap){
                resize_shared_memory(capacity(), 2 * capacity());
            }
            if (std::is_copy_assignable_v<T>) {
                *m_end = ele;
            } else if (std::is_copy_constructible_v<T>) {
                *m_end = T{ele};
            }
            m_end++;
            return m_end-1-m_begin;
        }

    private:
        T* m_begin;
        T* m_end;
        T* m_end_cap;
        int m_shm_fd;
        size_t file_size;
        const size_t m_page_aligned_vec_size = sysconf(_SC_PAGESIZE) / sizeof(T);

    };
}

#endif //DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
