/*
   +----------------------------------------------------------------------+
   | PHP Version 6                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stanislav Malyshev <stas@zend.com>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unicode/ustring.h>
#include <unicode/umsg.h>

#include "php_intl.h"
#include "msgformat_class.h"
#include "intl_data.h"

/* {{{ */
static void msgfmt_ctor(INTERNAL_FUNCTION_PARAMETERS) 
{
	char*       locale;
	int         locale_len = 0;
	UChar*      spattern     = NULL;
	int         spattern_len = 0;
	zval*       object;
	int free_pattern = 0;
	MessageFormatter_object* mfo;

	intl_error_reset( NULL TSRMLS_CC );
	object = return_value;
	/* Parse parameters. */
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "su",
		&locale, &locale_len, &spattern, &spattern_len ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"msgfmt_create: unable to parse input parameters", 0 TSRMLS_CC );
		zval_dtor(return_value);
		RETURN_NULL();
	}

	INTL_CHECK_LOCALE_LEN_OBJ(locale_len, return_value);
	MSG_FORMAT_METHOD_FETCH_OBJECT;

	/* Convert pattern (if specified) to UTF-16. */
	if(locale_len == 0) {
		locale = UG(default_locale);
	}

	(mfo)->mf_data.orig_format = eustrndup(spattern, spattern_len);
	(mfo)->mf_data.orig_format_len = spattern_len;
	
	if(msgformat_fix_quotes(&spattern, &spattern_len, &INTL_DATA_ERROR_CODE(mfo), &free_pattern) != SUCCESS) {
		INTL_CTOR_CHECK_STATUS(mfo, "msgfmt_create: error converting pattern to quote-friendly format");
	}

	/* Create an ICU message formatter. */
	MSG_FORMAT_OBJECT(mfo) = umsg_open(spattern, spattern_len, locale, NULL, &INTL_DATA_ERROR_CODE(mfo));
	if(free_pattern) {
		efree(spattern);
	}

	INTL_CTOR_CHECK_STATUS(mfo, "msgfmt_create: message formatter creation failed");
}
/* }}} */

/* {{{ proto MessageFormatter MesssageFormatter::create( string $locale, string $pattern )
 * Create formatter. }}} */
/* {{{ proto MessageFormatter msgfmt_create( string $locale, string $pattern )
 * Create formatter.
 */
PHP_FUNCTION( msgfmt_create )
{
	object_init_ex( return_value, MessageFormatter_ce_ptr );
	msgfmt_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void MessageFormatter::__construct( string $locale, string $pattern )
 * MessageFormatter object constructor.
 */
PHP_METHOD( MessageFormatter, __construct )
{
	return_value = getThis();
	msgfmt_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int MessageFormatter::getErrorCode()
 * Get formatter's last error code. }}} */
/* {{{ proto int msgfmt_get_error_code( MessageFormatter $nf )
 * Get formatter's last error code.
 */
PHP_FUNCTION( msgfmt_get_error_code )
{
	zval*                    object  = NULL;
	MessageFormatter_object*  mfo     = NULL;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, MessageFormatter_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"msgfmt_get_error_code: unable to parse input params", 0 TSRMLS_CC );

		RETURN_FALSE;
	}

	mfo = (MessageFormatter_object *) zend_object_store_get_object( object TSRMLS_CC );

	/* Return formatter's last error code. */
	RETURN_LONG( INTL_DATA_ERROR_CODE(mfo) );
}
/* }}} */

/* {{{ proto string MessageFormatter::getErrorMessage( )
 * Get text description for formatter's last error code. }}} */
/* {{{ proto string msgfmt_get_error_message( MessageFormatter $coll )
 * Get text description for formatter's last error code.
 */
PHP_FUNCTION( msgfmt_get_error_message )
{
	char*                    message = NULL;
	zval*                    object  = NULL;
	MessageFormatter_object*  mfo     = NULL;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, MessageFormatter_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"msgfmt_get_error_message: unable to parse input params", 0 TSRMLS_CC );

		RETURN_FALSE;
	}

	mfo = (MessageFormatter_object *) zend_object_store_get_object( object TSRMLS_CC );

	/* Return last error message. */
	message = intl_error_get_message( &mfo->mf_data.error TSRMLS_CC );
	RETURN_STRING( message, 0);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */