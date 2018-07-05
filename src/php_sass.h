/**
 * Sass
 * PHP bindings to libsass - fast, native Sass parsing in PHP!
 *
 * https://github.com/absalomedia/sassphp
 * Copyright (c)2012 Jamie Rumbelow <http://jamierumbelow.net>
 * Copyright (c)2017 Lawrence Meckan <http://absalom.biz>
 */

#ifndef PHP_SASS_H
#define PHP_SASS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define SASS_VERSION "0.5.16"
#define SASS_FLAVOR "Sassyphpras"

#include <php.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_exceptions.h>

#include <sass.h>
#include <sass2scss.h>

zend_class_entry *sass_ce;
zend_class_entry *sass_exception_ce;

zend_class_entry *sass_get_exception_base();

#ifdef PHP_WIN32
#define PHP_SASS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_SASS_API __attribute__((visibility("default")))
#else
#define PHP_SASS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_METHOD(Sass, __construct);
PHP_METHOD(Sass, compile);
PHP_METHOD(Sass, compileFile);
PHP_METHOD(Sass, getStyle);
PHP_METHOD(Sass, setStyle);
PHP_METHOD(Sass, getIncludePath);
PHP_METHOD(Sass, setIncludePath);
PHP_METHOD(Sass, getPrecision);
PHP_METHOD(Sass, setPrecision);
PHP_METHOD(Sass, getComments);
PHP_METHOD(Sass, setComments);
PHP_METHOD(Sass, getIndent);
PHP_METHOD(Sass, setIndent);
PHP_METHOD(Sass, getEmbed);
PHP_METHOD(Sass, setEmbed);
PHP_METHOD(Sass, getMapPath);
PHP_METHOD(Sass, setMapPath);
PHP_METHOD(Sass, getMapRoot);
PHP_METHOD(Sass, setMapRoot);

#endif
