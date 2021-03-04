//
// Created by Martin Boros on 3/1/21.
//

#ifndef DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
#define DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H

namespace dtl {
#include <sys/mman.h>
#include <unistd.h>

    // TODO: make a metadata class so it's easier to manage the pointer math
    // TODO: look into how to add a lock for synchronization to metadata
    // TODO: figure out how to pass by reference for threads; copy is messing it up
    // TODO: add a reference counter to metadata so it's only unmapped on last free
    template<typename T>
    class DistributedVector {
    private:
        ///
        /// \param size size_t size of shared memory; must be page aligned
        /// \return pointer to allocated shared memory
        void create_shared_memory(size_t size) {
            int protection = PROT_READ | PROT_WRITE;
            int visibility = MAP_SHARED;

            m_shm_fd = memfd_create("shm", 0);
            ftruncate(m_shm_fd, size * sizeof(T));

            void* result = mmap(nullptr, size * sizeof(T)+2*sizeof(size_t), protection, visibility, m_shm_fd, 0);
            if (result == MAP_FAILED){
                throw std::runtime_error("Memory mapping failed");
            }

            m_end_cap = static_cast<size_t*>(result);
            m_end = m_end_cap+1;
            result = static_cast<char*>(result)+2*sizeof(size_t);
            m_begin = static_cast<T*>(result);
        }

        ///
        /// \param size size_t current size of shared memory
        /// \param new_size size_t desired size of new memory
        void resize_shared_memory() {
            // current size is size of current array + metadata
            size_t current_size = size()*sizeof(T) + 2*sizeof(size_t);
            if (current_size % sysconf(_SC_PAGESIZE) != 0){
                throw std::invalid_argument("size must be page aligned");
            }
            size_t new_size = 2*current_size;

            void* result = mremap(m_end_cap, current_size, new_size, MREMAP_MAYMOVE);
            if (result == MAP_FAILED){
                throw std::runtime_error("Memory remapping failed");
            }
            ftruncate(m_shm_fd, new_size);
            m_end_cap = static_cast<size_t*>(result);
            *m_end_cap = (new_size-2*sizeof(size_t))/sizeof(T);
            m_end = m_end_cap+1;
            result = static_cast<char*>(result)+2*sizeof(size_t);
            m_begin = static_cast<T*>(result);
        }

        ///
        /// checks for invariant type T; must be copyable so it can be copied into shared memory
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
            create_shared_memory(m_page_aligned_vec_size);
            *m_end_cap = m_page_aligned_vec_size;
            *m_end = 0;
        }

        DistributedVector(size_t size){
            check_type_invariants();
            size_t alloc_size = m_page_aligned_vec_size + 2*sizeof(size_t)/sizeof(T);
            while(alloc_size <= 2*size){
                alloc_size *= 2;
            }
            alloc_size -= 2*sizeof(size_t)/sizeof(T);
            create_shared_memory(alloc_size);
            *m_end_cap = alloc_size;
            *m_end = size;
        }

        ~DistributedVector(){
//            munmap(m_begin, *m_end_cap);
        }

        ///
        /// \return size_t size of array in use
        size_t size() {
            return *m_end;
        }

        ///
        /// \return size_t capacity of allocated shared memory
        size_t capacity() {
            return *m_end_cap;
        }

        ///
        /// \return Pointer to underlying shared memory array
        T* data(){
            return m_begin;
        }

        ///
        /// \param index size_t index of member to access; bounds checking included, throws out_of_range exception if attempted access outside of array bounds
        /// \return reference to member at index
        T& at(size_t index){
            if (index < size()) {
                return *(m_begin+index);
            } else {
                throw std::out_of_range("Array bound exception");
            }
        }

        ///
        /// \param ele element to be added to the vector; ele is copied
        /// \return index where ele was added
        size_t push_back(const T& ele){
            if (size() == capacity()){
                resize_shared_memory();
            }
            if (std::is_copy_assignable_v<T>) {
                *(m_begin+size())= ele;
            } else if (std::is_copy_constructible_v<T>) {
                *(m_begin+size()) = T{ele};
            }
            (*m_end)++;
            return size()-1;
        }

    private:
        T* m_begin;
        size_t* m_end;
        size_t* m_end_cap;
        int m_shm_fd;
        int m_shm_metadata_fd;
        const size_t m_page_aligned_vec_size = (sysconf(_SC_PAGESIZE)-sizeof(size_t)*2) / sizeof(T);

    };
}

#endif //DISTRIBUTEDTEMPLATELIBRARY_DISTRIBUTEDVECTOR_H
