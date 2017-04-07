/**
 * Sass
 * PHP bindings to libsass - fast, native Sass parsing in PHP!
 *
 * https://github.com/jamierumbelow/sassphp
 * Copyright (c)2012 Jamie Rumbelow <http://jamierumbelow.net>
 *
 * Fork updated and maintained by https://github.com/absalomedia
 */

#include <stdio.h>
#if ZEND_MODULE_API_NO > 20131226
#include <stdlib.h>.
#endif

#include "php_sass.h"
#include "utilities.h"

/* --------------------------------------------------------------
 * Sass
 * ------------------------------------------------------------ */

zend_object_handlers sass_handlers;

typedef struct sass_object {
    #if PHP_MAJOR_VERSION < 7
    zend_object zo;
    #endif
    int style;
    char* include_paths;
    bool comments;
    bool indent;
    long precision;
    char* map_path;
    bool omit_map_url;
    bool map_embed;
    bool map_contents;
    char* map_root;
    #if PHP_MAJOR_VERSION >= 7
    zend_object zo;
    #endif
} sass_object;

zend_class_entry *sass_ce;

#if PHP_MAJOR_VERSION >= 7

static inline sass_object *sass_fetch_object(zend_object *obj)
{
    return (sass_object *) ((char*) (obj) - XtOffsetOf(sass_object, zo));
}

#define Z_SASS_P(zv) sass_fetch_object(Z_OBJ_P((zv)));

static void sass_free_storage(zend_object *object TSRMLS_DC)
{
    sass_object *obj;
    obj = sass_fetch_object(object);
    zend_object_std_dtor(object TSRMLS_DC);

}
#else
void sass_free_storage(void *object TSRMLS_DC)
{
    sass_object *obj = (sass_object *)object;
    if (obj->include_paths != NULL)
        efree(obj->include_paths);
    if (obj->map_path != NULL)
        efree(obj->map_path);
    if (obj->map_root != NULL)
        efree(obj->map_root);
    zend_hash_destroy(obj->zo.properties);
    FREE_HASHTABLE(obj->zo.properties);
    efree(obj);
}
#endif


#if ZEND_MODULE_API_NO <= 20131226
zend_object_value sass_create_handler(zend_class_entry *type TSRMLS_DC) {
    zval *tmp;
    zend_object_value retval;

    sass_object *obj = (sass_object *)emalloc(sizeof(sass_object));
    memset(obj, 0, sizeof(sass_object));

    obj->zo.ce = type;

    ALLOC_HASHTABLE(obj->zo.properties);
    zend_hash_init(obj->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID > 50399
    object_properties_init(&(obj->zo), type);
#endif

    retval.handle = zend_objects_store_put(obj, NULL,
        sass_free_storage, NULL TSRMLS_CC);
    retval.handlers = &sass_handlers;

    return retval;
 }
#endif

#if ZEND_MODULE_API_NO > 20131226
zend_object * sass_create_handler(zend_class_entry *type TSRMLS_DC) {
    struct sass_object *obj = ecalloc(1,
         sizeof(struct sass_object) +
         zend_object_properties_size(type));

     zend_object_std_init(&obj->zo, type TSRMLS_CC);
     object_properties_init(&obj->zo, type TSRMLS_CC);

     obj->zo.handlers = &sass_handlers;

     return &obj->zo;
}

#endif


PHP_METHOD(Sass, __construct)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_NULL();
    }
    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->style = SASS_STYLE_NESTED;
    obj->include_paths = NULL;
    obj->map_path = NULL;
    obj->map_root = NULL;
    obj->comments = false;
    obj->indent = false;
    obj->map_embed = false;
    obj->map_contents = false;
    obj->omit_map_url = true;
    obj->precision = 5;

}


void set_options(sass_object *this, struct Sass_Context *ctx)
{
    struct Sass_Options* opts = sass_context_get_options(ctx);

    sass_option_set_precision(opts, this->precision);
    sass_option_set_output_style(opts, this->style);
    sass_option_set_is_indented_syntax_src(opts, this->indent);
    if (this->include_paths != NULL) {
    sass_option_set_include_path(opts, this->include_paths);
    }
    sass_option_set_source_comments(opts, this->comments);
    if (this->comments) {
    sass_option_set_omit_source_map_url(opts, false);
    }
    sass_option_set_source_map_embed(opts, this->map_embed);
    sass_option_set_source_map_contents(opts, this->map_contents);
    if (this->map_path != NULL) {
    sass_option_set_source_map_file(opts, this->map_path);
    sass_option_set_omit_source_map_url(opts, false);
    sass_option_set_source_map_contents(opts, true);
    }
    if (this->map_root != NULL) {
    sass_option_set_source_map_root(opts, this->map_root);
    }

}

/**
 * $sass->parse(string $source, [  ]);
 *
 * Parse a string of Sass; a basic input -> output affair.
 */
PHP_METHOD(Sass, compile)
{
    #if ZEND_MODULE_API_NO > 20131226
    sass_object *this = Z_SASS_P(getThis());
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *this = (sass_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    #endif

    // Define our parameters as local variables
    char *source;
    #if ZEND_MODULE_API_NO <= 20131226
    int source_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t source_len;
    #endif

    // Use zend_parse_parameters() to grab our source from the function call
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &source, &source_len) == FAILURE){
        RETURN_FALSE;
    }

    // Create a new sass_context
    struct Sass_Data_Context* data_context = sass_make_data_context(strdup(source));

    struct Sass_Context* ctx = sass_data_context_get_context(data_context);

    set_options(this, ctx);

    int status = sass_compile_data_context(data_context);

    // Check the context for any errors...
    if (status != 0)
    {
        zend_throw_exception(sass_exception_ce, sass_context_get_error_message(ctx), 0 TSRMLS_CC);
    }
    else
    {
        #if ZEND_MODULE_API_NO <= 20131226
        RETVAL_STRING(sass_context_get_output_string(ctx), 1);
        #endif
        #if ZEND_MODULE_API_NO > 20131226
        RETVAL_STRING(sass_context_get_output_string(ctx));
        #endif
    }

    sass_delete_data_context(data_context);
}

/**
 * $sass->parse_file(string $file_name);
 *
 * Parse a whole file FULL of Sass and return the CSS output
 */
PHP_METHOD(Sass, compileFile)
{
    array_init(return_value);

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *this = Z_SASS_P(getThis());
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *this = (sass_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    #endif

    // We need a file name and a length
    char *file;
    #if ZEND_MODULE_API_NO <= 20131226
    int file_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t file_len;
    #endif

    // Grab the file name from the function
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    // First, do a little checking of our own. Does the file exist?
    if( access( file, F_OK ) == -1 )
    {
        zend_throw_exception_ex(sass_exception_ce, 0 TSRMLS_CC, "File %s could not be found", file);
        RETURN_FALSE;
    }

    struct Sass_File_Context* file_ctx = sass_make_file_context(file);

    struct Sass_Context* ctx = sass_file_context_get_context(file_ctx);

    set_options(this, ctx);

    int status = sass_compile_file_context(file_ctx);

    // Check the context for any errors...
    if (status != 0)
    {
        zend_throw_exception(sass_exception_ce, sass_context_get_error_message(ctx), 0 TSRMLS_CC);
    }
    else
    {

        #if ZEND_MODULE_API_NO <= 20131226
        if (this->map_path != NULL ) {
        // Send it over to PHP.
        add_next_index_string(return_value, sass_context_get_output_string(ctx), 1);
        } else {
        RETVAL_STRING(sass_context_get_output_string(ctx), 1);
        }

         // Do we have source maps to go?
         if (this->map_path != NULL)
         {
         // Send it over to PHP.
         add_next_index_string(return_value, sass_context_get_source_map_string(ctx), 1);
         }
         #endif

        #if ZEND_MODULE_API_NO > 20131226
        if (this->map_path != NULL ) {
        // Send it over to PHP.
        add_next_index_string(return_value, sass_context_get_output_string(ctx));
        } else {
        RETVAL_STRING(sass_context_get_output_string(ctx));
        }

         // Do we have source maps to go?
         if (this->map_path != NULL)
         {
         // Send it over to PHP.
         add_next_index_string(return_value, sass_context_get_source_map_string(ctx));
         }
         #endif
    }

    sass_delete_file_context(file_ctx);
}

PHP_METHOD(Sass, getStyle)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->style);
}

PHP_METHOD(Sass, setStyle)
{
    zval *this = getThis();

    long new_style;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &new_style) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif
    obj->style = new_style;

    RETURN_NULL();
}

PHP_METHOD(Sass, getIncludePath)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->include_paths == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->include_paths, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->include_paths == NULL) RETURN_STRING("");
    RETURN_STRING(obj->include_paths);
    #endif
}

PHP_METHOD(Sass, setIncludePath)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif    

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->include_paths != NULL)
        efree(obj->include_paths);
    obj->include_paths = estrndup(path, path_len);

    RETURN_NULL();
}

PHP_METHOD(Sass, getMapPath)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->map_path == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->map_path, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->map_path == NULL) RETURN_STRING("");
    RETURN_STRING(obj->map_path);
    #endif
}

PHP_METHOD(Sass, setMapPath)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->map_path != NULL)
        efree(obj->map_path);
    obj->map_path = estrndup(path, path_len);

    RETURN_NULL();
}


PHP_METHOD(Sass, getMapRoot)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    if (obj->map_root == NULL) RETURN_STRING("", 1);
    RETURN_STRING(obj->map_root, 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    if (obj->map_root == NULL) RETURN_STRING("");
    RETURN_STRING(obj->map_root);
    #endif
}

PHP_METHOD(Sass, setMapRoot)
{
    zval *this = getThis();

    char *path;
    #if ZEND_MODULE_API_NO <= 20131226
    int path_len;
    #endif
    #if ZEND_MODULE_API_NO > 20131226
    size_t path_len;
    #endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
        RETURN_FALSE;

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    if (obj->map_root != NULL)
        efree(obj->map_root);
    obj->map_path = estrndup(path, path_len);

    RETURN_NULL();
}




PHP_METHOD(Sass, getPrecision)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->precision);
}

PHP_METHOD(Sass, setPrecision)
{
    zval *this = getThis();

    long new_precision;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &new_precision) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->precision = new_precision;

    RETURN_NULL();
}

PHP_METHOD(Sass, getEmbed)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->map_embed);
}

PHP_METHOD(Sass, setEmbed)
{
    zval *this = getThis();

    bool new_map_embed;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_map_embed) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->map_embed = new_map_embed;

    RETURN_NULL();
}


PHP_METHOD(Sass, getComments)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->comments);
}

PHP_METHOD(Sass, setComments)
{
    zval *this = getThis();

    bool new_comments;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_comments) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->comments = new_comments;

    RETURN_NULL();
}


PHP_METHOD(Sass, getIndent)
{
    zval *this = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    RETURN_LONG(obj->indent);
}

PHP_METHOD(Sass, setIndent)
{
    zval *this = getThis();

    bool new_indent;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &new_indent) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO > 20131226
    sass_object *obj = Z_SASS_P(this);
    #endif
    #if ZEND_MODULE_API_NO <= 20131226
    sass_object *obj = (sass_object *)zend_object_store_get_object(this TSRMLS_CC);
    #endif

    obj->indent = new_indent;

    RETURN_NULL();
}


PHP_METHOD(Sass, getLibraryVersion)
{
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", NULL) == FAILURE) {
        RETURN_FALSE;
    }

    #if ZEND_MODULE_API_NO <= 20131226
    RETURN_STRING(libsass_version(), 1);
    #endif

    #if ZEND_MODULE_API_NO > 20131226
    RETURN_STRING(libsass_version());
    #endif

}
/* --------------------------------------------------------------
 * EXCEPTION HANDLING
 * ------------------------------------------------------------ */

zend_class_entry *sass_get_exception_base(TSRMLS_D)
{
    return zend_exception_get_default(TSRMLS_C);
}

/* --------------------------------------------------------------
 * PHP EXTENSION INFRASTRUCTURE
 * ------------------------------------------------------------ */

ZEND_BEGIN_ARG_INFO(arginfo_sass_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_compile, 0, 0, 1)
    ZEND_ARG_INFO(0, sass_string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_compileFile, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setStyle, 0, 0, 1)
    ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setIncludePath, 0, 0, 1)
    ZEND_ARG_INFO(0, include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setPrecision, 0, 0, 1)
    ZEND_ARG_INFO(0, precision)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setComments, 0, 0, 1)
    ZEND_ARG_INFO(0, comments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setIndent, 0, 0, 1)
    ZEND_ARG_INFO(0, indent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setEmbed, 0, 0, 1)
    ZEND_ARG_INFO(0, map_embed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setMapPath, 0, 0, 1)
    ZEND_ARG_INFO(0, map_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sass_setMapRoot, 0, 0, 1)
    ZEND_ARG_INFO(0, map_root)
ZEND_END_ARG_INFO()
    
zend_function_entry sass_methods[] = {
    PHP_ME(Sass,  __construct,       arginfo_sass_void,           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Sass,  compile,           arginfo_sass_compile,        ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  compileFile,       arginfo_sass_compileFile,    ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getStyle,          arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setStyle,          arginfo_sass_setStyle,       ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getIncludePath,    arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setIncludePath,    arginfo_sass_setIncludePath, ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getPrecision,      arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setPrecision,      arginfo_sass_setPrecision,   ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getComments,       arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setComments,       arginfo_sass_setComments,    ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getIndent,         arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setIndent,         arginfo_sass_setIndent,      ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getEmbed,          arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setEmbed,          arginfo_sass_setEmbed,       ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getMapPath,        arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setMapPath,        arginfo_sass_setMapPath,     ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getMapRoot,        arginfo_sass_void,           ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  setMapRoot,        arginfo_sass_setMapRoot,     ZEND_ACC_PUBLIC)
    PHP_ME(Sass,  getLibraryVersion, arginfo_sass_void,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(Sass, compile_file, compileFile, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};


static PHP_MINIT_FUNCTION(sass)
{
    zend_class_entry ce;
    zend_class_entry exception_ce;

    INIT_CLASS_ENTRY(ce, "Sass", sass_methods);

    sass_ce = zend_register_internal_class(&ce TSRMLS_CC);
    sass_ce->create_object = sass_create_handler;

    memcpy(&sass_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    #if ZEND_MODULE_API_NO > 20131226
    sass_handlers.offset = XtOffsetOf(struct sass_object, zo);
    sass_handlers.free_obj = sass_free_storage;
    #endif
    sass_handlers.clone_obj = NULL;

    INIT_CLASS_ENTRY(exception_ce, "SassException", NULL);

    #if ZEND_MODULE_API_NO > 20131226
    sass_exception_ce = zend_register_internal_class_ex(&exception_ce, sass_get_exception_base(TSRMLS_C));
    #endif

    #if ZEND_MODULE_API_NO <= 20131226
    sass_exception_ce = zend_register_internal_class_ex(&exception_ce, sass_get_exception_base(TSRMLS_C), NULL TSRMLS_CC);
    #endif

    #define REGISTER_SASS_CLASS_CONST_LONG(name, value) zend_declare_class_constant_long(sass_ce, ZEND_STRS( #name ) - 1, value TSRMLS_CC)

    REGISTER_SASS_CLASS_CONST_LONG(STYLE_NESTED, SASS_STYLE_NESTED);
    REGISTER_SASS_CLASS_CONST_LONG(STYLE_EXPANDED, SASS_STYLE_EXPANDED);
    REGISTER_SASS_CLASS_CONST_LONG(STYLE_COMPACT, SASS_STYLE_COMPACT);
    REGISTER_SASS_CLASS_CONST_LONG(STYLE_COMPRESSED, SASS_STYLE_COMPRESSED);

    REGISTER_STRING_CONSTANT("SASS_FLAVOR", SASS_FLAVOR, CONST_CS | CONST_PERSISTENT);


    return SUCCESS;
}

static PHP_MINFO_FUNCTION(sass)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "sass support", "enabled");
    php_info_print_table_row(2, "version", SASS_VERSION);
    php_info_print_table_row(2, "flavor", SASS_FLAVOR);
    php_info_print_table_row(2, "libsass version", libsass_version());
    php_info_print_table_end();
}

static zend_module_entry sass_module_entry = {
    STANDARD_MODULE_HEADER,
    "sass",
    NULL,
    PHP_MINIT(sass),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(sass),
    SASS_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SASS
ZEND_GET_MODULE(sass)
#endif
