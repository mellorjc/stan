#ifndef STAN_LANG_AST_LOCAL_VAR_TYPE_HPP
#define STAN_LANG_AST_LOCAL_VAR_TYPE_HPP

#include <stan/lang/ast/node/expression.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <string>

namespace stan {
  namespace lang {

    //TODO:mitzi - need visitor to get size of local var container types

    /** 
     * Local variable types have sized container types.
     */
    struct local_array_type;
    struct double_type;
    struct ill_formed_type;
    struct int_type;
    struct matrix_local_type;
    struct row_vector_local_type;
    struct vector_local_type;

    struct local_var_type {
      /**
       * Recursive wrapper for local variable types.
       */
      typedef boost::variant<
        boost::recursive_wrapper<ill_formed_type>,
        boost::recursive_wrapper<double_type>,
        boost::recursive_wrapper<int_type>,
        boost::recursive_wrapper<matrix_local_type>,
        boost::recursive_wrapper<row_vector_local_type>,
        boost::recursive_wrapper<vector_local_type>,
        boost::recursive_wrapper<local_array_type> >
      local_t;

      /**
       * The local variable type held by this wrapper.
       */
      local_t var_type_;

      /**
       * Construct a bare var type with default values.
       */
      local_var_type();

      /**
       * Construct a local var type 
       *
       * @param type local variable type raw variant type.
       */
      local_var_type(const local_var_type& type);  // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const ill_formed_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const double_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      

      local_var_type(const int_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const matrix_local_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const row_vector_local_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const vector_local_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */      
      local_var_type(const local_array_type& type); // NOLINT(runtime/explicit)

      /**
       * Construct a local var type with the specified type.
       *
       * @param type local variable type
       */
      local_var_type(const local_t& var_type_);  // NOLINT(runtime/explicit)

      /**
       * If `var_type` is `local_array_type`, returns the innermost type
       * contained in the array, otherwise will return `ill_formed_type`.
       */
      local_var_type array_contains() const;

      /**
       * Returns number of array dimensions for this type.
       * Returns 0 for non-array types.
       */
      int array_dims() const;

      /**
       * Returns array element type if `var_type_` is `local_array_type`,
       * ill_formed_type otherwise.  (Call `is_array_type()` first.)
       */
      local_var_type array_element_type() const;

      /**
       * Returns array length for local_array_type, nil otherwise.
       */
      expression array_len() const;

      /**
       * Returns equivalent bare_expr_type (unsized) for this local type.
       */
      bare_expr_type bare_type() const;
      
      /**
       * Returns true if `var_type_` is `local_array_type`, false otherwise.
       */
      bool is_array_type() const;

      /**
       * Returns total number of dimensions for container type.
       * Returns 0 for scalar types.
       */
      int num_dims() const;

      /**
       * Returns vector of sizes for each dimension, empty vector if unsized.
       */
      std::vector<expression> size() const;
    };
    
  }
}
#endif