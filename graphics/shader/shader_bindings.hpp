

#pragma once

#include "binding_point.h"
#include "../types.h"
#include "../buffer/vertex_attribute.h"

namespace motor
{
    namespace graphics
    {
        //******************************************************************
        class vertex_input_binding
        {

            motor_this_typedefs( vertex_input_binding ) ;

        private:

            motor::string_t _name ;
            motor::graphics::vertex_attribute _va = motor::graphics::vertex_attribute::undefined ;

        public:

            vertex_input_binding( void_t ) noexcept {}
            vertex_input_binding( motor::string_cref_t name_,
                motor::graphics::vertex_attribute const va_ ) noexcept : _name( name_ ), _va( va_ ) {}
            vertex_input_binding( this_cref_t rhv ) noexcept : _name( rhv._name ), _va( rhv._va ) {}
            vertex_input_binding( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ), _va( rhv._va ) {}
            ~vertex_input_binding( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name; _va = rhv._va ; return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ; _va = rhv._va ; return *this ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

            motor::graphics::vertex_attribute va( void_t ) const noexcept
            {
                return _va ;
            }

            void_t set( motor::graphics::vertex_attribute const va ) noexcept
            {
                _va = va ;
            }

            void_t set( motor::string_in_t const name ) noexcept
            {
                _name = name ;
            }

            void_t set( motor::string_rref_t const name ) noexcept
            {
                _name = std::move( name ) ;
            }
        };
        motor_typedef( vertex_input_binding ) ;

        //******************************************************************
        class vertex_output_binding
        {
            motor_this_typedefs( vertex_output_binding ) ;

        private:

            motor::string_t _name ;
            motor::graphics::vertex_attribute _va = motor::graphics::vertex_attribute::undefined ;
            motor::graphics::ctype _ct ;

        public:

            vertex_output_binding( void_t ) noexcept {}
            vertex_output_binding( motor::string_cref_t name_,
                motor::graphics::vertex_attribute const va_ ) noexcept : _name( name_ ), _va( va_ ),
                _ct( motor::graphics::deduce_from( va_ ) ) {}
            vertex_output_binding( motor::string_cref_t name_,
                motor::graphics::vertex_attribute const va_, motor::graphics::ctype const ct_ ) noexcept : _name( name_ ), _va( va_ ), _ct( ct_ ) {}
            vertex_output_binding( this_cref_t rhv ) noexcept { *this = rhv ; }
            vertex_output_binding( this_rref_t rhv ) noexcept { *this = std::move( rhv ) ; }
            ~vertex_output_binding( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name; _va = rhv._va ; _ct = rhv._ct ; return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ; _va = rhv._va ; _ct = std::move( rhv._ct ) ; return *this ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

            motor::graphics::vertex_attribute va( void_t ) const noexcept
            {
                return _va ;
            }

            motor::graphics::ctype ctype( void_t ) const noexcept
            {
                return _ct ;
            }

            void_t set( motor::graphics::ctype const ct ) noexcept
            {
                _ct = ct ;
            }

            void_t set( motor::graphics::vertex_attribute const va ) noexcept
            {
                _va = va ;
            }

            void_t set( motor::string_in_t const name ) noexcept
            {
                _name = name ;
            }

            void_t set( motor::string_rref_t const name ) noexcept
            {
                _name = std::move( name ) ;
            }
            
        };
        motor_typedef( vertex_output_binding ) ;

        //******************************************************************
        class variable_binding
        {
            motor_this_typedefs( variable_binding ) ;

        private:

            motor::string_t _name ;
            motor::graphics::binding_point _bp ;

        public:

            variable_binding( void_t ) noexcept {}
            variable_binding( motor::string_in_t name, motor::graphics::binding_point bp ) noexcept :
                _name( name ), _bp( bp ) {}
            variable_binding( this_cref_t rhv ) noexcept : _name( rhv._name ), _bp( rhv._bp ) {}
            variable_binding( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ), _bp( rhv._bp ) {}
            ~variable_binding( void_t ) noexcept {}

        public:

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name ;
                _bp = rhv._bp ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;
                _bp = rhv._bp ;
                return *this ;
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept { return _name ; }
            motor::graphics::binding_point bp( void_t ) const noexcept{ return _bp ; }
        };
        motor_typedef( variable_binding ) ;

        //******************************************************************
        class shader_bindings
        {
            motor_this_typedefs( shader_bindings ) ;

        private:

            motor::vector< motor::graphics::vertex_input_binding > _vertex_inputs ;
            motor::vector< motor::graphics::vertex_output_binding > _vertex_outputs ;
            motor::vector< motor::graphics::variable_binding > _variable_bindings ;

        public:

            shader_bindings( void_t ) noexcept{}
            
            shader_bindings( this_cref_t rhv ) noexcept : 
                _vertex_inputs( rhv._vertex_inputs ), _vertex_outputs( rhv._vertex_outputs ),
                _variable_bindings( rhv._variable_bindings ) {}
            
            shader_bindings( this_rref_t rhv ) noexcept :
                _vertex_inputs( std::move( rhv._vertex_inputs ) ), _vertex_outputs( std::move( rhv._vertex_outputs ) ),
                _variable_bindings( std::move( rhv._variable_bindings ) ) {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _vertex_inputs = rhv._vertex_inputs ;
                _vertex_outputs = rhv._vertex_outputs ;
                _variable_bindings = rhv._variable_bindings ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _vertex_inputs = std::move( rhv._vertex_inputs )  ;
                _vertex_outputs = std::move( rhv._vertex_outputs ) ;
                _variable_bindings = std::move( rhv._variable_bindings ) ;
                return *this ;
            }

            ~shader_bindings( void_t ) noexcept {}

            void_t clear( void_t ) noexcept
            {
                _vertex_inputs.clear() ;
                _vertex_outputs.clear() ;
                _variable_bindings.clear() ;
            }

        public:

            typedef std::function < void_t ( size_t const i,
                motor::graphics::vertex_attribute const va, motor::string_cref_t name ) > for_each_vib_funk_t ;
            
            //**************************************************************************
            void_t for_each_vertex_input_binding( for_each_vib_funk_t funk ) const noexcept
            {
                size_t i = 0 ;
                for( auto const & vib : _vertex_inputs ) funk( i++, vib.va(), vib.name() ) ;
                
            }

            typedef std::function < void_t ( size_t const i,
                motor::graphics::vertex_attribute const va, motor::graphics::ctype const, motor::string_cref_t name ) > for_each_vob_funk_t ;

            //**************************************************************************
            void_t for_each_vertex_output_binding( for_each_vob_funk_t funk ) const noexcept
            {
                size_t i = 0 ;
                for( auto const & vb : _vertex_outputs ) funk( i++, vb.va(), vb.ctype(), vb.name() ) ;
            }

            //**************************************************************************
            this_ref_t add_vertex_input_binding( motor::graphics::vertex_attribute const va,
                motor::string_cref_t name ) noexcept
            {
                auto iter = std::find_if( _vertex_inputs.begin(), _vertex_inputs.end(),
                    [&] ( motor::graphics::vertex_input_binding_in_t b )
                {
                    return b.name() == name ;
                } ) ;

                if( iter == _vertex_inputs.end() )
                {
                    iter = std::find_if( _vertex_inputs.begin(), _vertex_inputs.end(),
                        [&] ( motor::graphics::vertex_input_binding_in_t b )
                    {
                        return b.va() == va ;
                    } ) ;
                }

                if( iter == _vertex_inputs.end() )
                {
                    _vertex_inputs.push_back( motor::graphics::vertex_input_binding( name, va ) ) ;
                    return *this ;
                }
                iter->set( va ) ;
                return *this ;
            }

            //**************************************************************************
            this_ref_t add_vertex_output_binding( motor::graphics::vertex_attribute const va,
                motor::string_cref_t name ) noexcept
            {
                auto iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                    [&] ( motor::graphics::vertex_output_binding_in_t b )
                {
                    return b.name() == name ;
                } ) ;

                if( iter == _vertex_outputs.end() )
                {
                    iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                        [&] ( motor::graphics::vertex_output_binding_in_t b )
                    {
                        return b.va() == va ;
                    } ) ;
                }

                if( iter == _vertex_outputs.end() )
                {
                    _vertex_outputs.push_back( motor::graphics::vertex_output_binding( name, va ) ) ;
                    return *this ;
                }
                iter->set( va ) ;
                return *this ;
            }

            //**************************************************************************
            this_ref_t add_vertex_output_binding( motor::graphics::vertex_attribute const va, 
                motor::graphics::ctype const ct, motor::string_cref_t name ) noexcept
            {
                auto iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                    [&] ( motor::graphics::vertex_output_binding_in_t b )
                {
                    return b.name() == name ;
                } ) ;

                if( iter == _vertex_outputs.end() )
                {
                    iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                        [&] ( motor::graphics::vertex_output_binding_in_t b )
                    {
                        return b.va() == va ;
                    } ) ;
                }

                if( iter == _vertex_outputs.end() )
                {
                    _vertex_outputs.push_back( motor::graphics::vertex_output_binding( name, va, ct ) ) ;
                    return *this ;
                }
                iter->set( va ) ;
                iter->set( motor::graphics::deduce_from( va ) ) ;
                return *this ;
            }

            //**************************************************************************
            bool_t find_vertex_input_binding_by_name( motor::string_cref_t name,
                motor::graphics::vertex_attribute & va ) const noexcept
            {
                auto iter = std::find_if( _vertex_inputs.begin(), _vertex_inputs.end(),
                    [&] ( motor::graphics::vertex_input_binding_in_t b )
                {
                    if( b.name() == name )
                    {
                        va = b.va() ;
                        return true ;
                    }
                    return false ;
                } ) ;

                return iter != _vertex_inputs.end() ;
            }

            //**************************************************************************
            bool_t find_vertex_output_binding_by_name( motor::string_cref_t name,
                motor::graphics::vertex_attribute & va ) const noexcept
            {
                auto iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                    [&] ( motor::graphics::vertex_output_binding_in_t b )
                {
                    if ( b.name() == name )
                    {
                        va = b.va() ;
                        return true ;
                    }
                    return false ;
                } ) ;

                return iter != _vertex_outputs.end() ;
            }

            //**************************************************************************
            bool_t find_vertex_input_binding_by_attribute( motor::graphics::vertex_attribute const va,
                motor::string_out_t name ) const noexcept
            {
                auto iter = std::find_if( _vertex_inputs.begin(), _vertex_inputs.end(),
                    [&] ( motor::graphics::vertex_input_binding_in_t b )
                {
                    if( b.va() == va ) 
                    {
                        name = b.name() ;
                        return true ;
                    }
                    return false ;
                } ) ;

                return iter != _vertex_inputs.end() ;
            }

            //**************************************************************************
            bool_t find_vertex_output_binding_by_attribute( motor::graphics::vertex_attribute const va,
                motor::string_out_t name ) const noexcept
            {
                auto iter = std::find_if( _vertex_outputs.begin(), _vertex_outputs.end(),
                    [&] ( motor::graphics::vertex_output_binding_in_t b )
                {
                    if( b.va() == va )
                    {
                        name = b.name() ;
                        return true ;
                    }

                    return false ;
                } ) ;

                return iter != _vertex_outputs.end() ;
            }

            //**************************************************************************
            this_ref_t add_input_binding( motor::graphics::binding_point const bp,
                motor::string_cref_t name ) noexcept
            {
                _variable_bindings.emplace_back( motor::graphics::variable_binding( name, bp ) ) ;
                return *this ;
            }

            //**************************************************************************
            bool_t has_input_binding( motor::graphics::binding_point const bp ) const noexcept
            {
                for( auto const & b : _variable_bindings )
                {
                    if( b.bp() == bp ) return true ;
                }
                return false ;
            }

            //**************************************************************************
            size_t get_num_output_bindings( void_t ) const noexcept
            {
                return _vertex_outputs.size() ;
            }
        };
        motor_typedef( shader_bindings ) ;
    }
}