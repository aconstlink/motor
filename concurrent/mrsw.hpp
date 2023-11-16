#pragma once

#include "semaphore.hpp"

namespace motor
{
    namespace concurrent
    {
        /// multi-reader single-writer
        class mrsw
        {
            motor_this_typedefs( mrsw ) ;

        private:

            semaphore_t _reader ;
            semaphore_t _writer ;

        public:

            mrsw( void_t ) noexcept
            {
            }

            mrsw( this_rref_t rhv ) noexcept :
                _reader( ::std::move( rhv._reader ) ),
                _writer( ::std::move( rhv._writer ) ) {}

            mrsw( this_cref_t ) = delete ;

            ~mrsw( void_t ) noexcept {}

        public:

            /// writer access to mrsw object
            class writer_lock
            {
                motor_this_typedefs( writer_lock ) ;

            private:

                mrsw& _l ;

            public:

                writer_lock( mrsw& r ) noexcept : _l( r )
                {
                    r.writer_increment() ;
                }

                ~writer_lock( void_t ) noexcept
                {
                    _l.writer_decrement() ;
                }

                writer_lock( this_cref_t ) = delete ;
                writer_lock( this_rref_t ) = delete ;

            };
            motor_typedef( writer_lock ) ;
            friend class writer_lock ;

        private: // writer

            void_t writer_increment( void_t ) noexcept
            {
                // ensure that only one writer is in critical section
                _writer.wait( 0, 1 ) ;

                // ensure that no reader is in critical section
                _reader.wait( 0, 0 ) ;
            }

            void_t writer_decrement( void_t )
            {
                _writer.decrement() ;
            }


        public:

            /// reader access to mrsw object
            class reader_lock
            {
                motor_this_typedefs( reader_lock ) ;

            private:

                mrsw& _l ;

            public:

                reader_lock( mrsw& r ) noexcept : _l( r )
                {
                    r.reader_increment() ;
                }

                ~reader_lock( void_t ) noexcept
                {
                    _l.reader_decrement() ;
                }

                reader_lock( this_cref_t ) = delete ;
                reader_lock( this_rref_t ) = delete ;

            };
            motor_typedef( reader_lock ) ;
            friend class reader_lock ;

        private: // reader

            void_t reader_increment( void_t ) noexcept
            {
                // just increment reader semaphore
                _reader.increment() ;

                // ensure that no writer is in critical section
                _writer.wait( 0, 0 ) ;
            }

            void_t reader_decrement( void_t )
            {
                _reader.decrement() ;
            }
        };
        motor_typedef( mrsw ) ;
    }
}