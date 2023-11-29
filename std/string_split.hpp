#pragma once

#include "string"
#include "vector"

namespace motor
{    
    namespace mstd
    {
        struct string_ops
        {
            static size_t split( motor::string_cref_t string_in, char deli,
                motor::vector<motor::string_t>& split_out )
            {
                size_t num_found = 0 ;
                size_t found_at = 0 ;
                size_t begin = 0 ;

                do
                {
                    found_at = string_in.find_first_of( deli, begin ) ;
                    if( begin < found_at )
                    {
                        split_out.push_back( string_in.substr( begin, found_at - begin ) ) ;
                        ++num_found ;
                    }
                    begin = found_at + 1 ;

                } while( found_at != std::string::npos ) ;

                if( num_found > 0 && split_out.back().size() == 0 )
                {
                    split_out.resize( split_out.size() - 1 ) ;
                }

                return num_found ;
            }
        };
    }
}