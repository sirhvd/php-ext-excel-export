/*
  +----------------------------------------------------------------------+
  | XlsWriter Extension                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2017-2018 The Viest                                    |
  +----------------------------------------------------------------------+
  | http://www.viest.me                                                  |
  +----------------------------------------------------------------------+
  | Author: viest <dev@service.viest.me>                                 |
  +----------------------------------------------------------------------+
*/

#include "xlswriter.h"

zend_class_entry *vtiful_xls_ce;

static zend_object_handlers vtiful_xls_handlers;

static zend_always_inline void *vtiful_object_alloc(size_t obj_size, zend_class_entry *ce) {
    void *obj = emalloc(obj_size + zend_object_properties_size(ce));
    memset(obj, 0, obj_size);
    return obj;
}

/* {{{ xls_objects_new
 */
PHP_VTIFUL_API zend_object *vtiful_xls_objects_new(zend_class_entry *ce)
{
    xls_object *intern = vtiful_object_alloc(sizeof(xls_object), ce);

    SHEET_LINE_INIT(intern)

    zend_object_std_init(&intern->zo, ce);
    object_properties_init(&intern->zo, ce);

    intern->zo.handlers = &vtiful_xls_handlers;

    return &intern->zo;
}
/* }}} */

/* {{{ vtiful_xls_objects_free
 */
static void vtiful_xls_objects_free(zend_object *object)
{
    xls_object *intern = php_vtiful_xls_fetch_object(object);

    lxw_workbook_free(intern->ptr.workbook);

    zend_object_std_dtor(&intern->zo);
}
/* }}} */

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(xls_construct_arginfo, 0, 0, 1)
                ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_file_name_arginfo, 0, 0, 1)
                ZEND_ARG_INFO(0, file_name)
                ZEND_ARG_INFO(0, sheet_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_const_memory_arginfo, 0, 0, 1)
                ZEND_ARG_INFO(0, file_name)
                ZEND_ARG_INFO(0, sheet_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_file_add_sheet, 0, 0, 1)
                ZEND_ARG_INFO(0, sheet_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_get_sheet_name, 0, 0, 1)
                ZEND_ARG_INFO(0, sheet_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_freeze_arginfo, 0, 0, 2)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_header_arginfo, 0, 0, 2)
                ZEND_ARG_INFO(0, header)
                ZEND_ARG_INFO(0, format_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_data_arginfo, 0, 0, 2)
                ZEND_ARG_INFO(0, data)
                ZEND_ARG_INFO(0, format_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_insert_text_arginfo, 0, 0, 4)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
                ZEND_ARG_INFO(0, data)
                ZEND_ARG_INFO(0, format_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_insert_url_arginfo, 0, 0, 4)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
                ZEND_ARG_INFO(0, url)
                ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_insert_chart_arginfo, 0, 0, 3)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
                ZEND_ARG_INFO(0, chart_resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_insert_image_arginfo, 0, 0, 3)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
                ZEND_ARG_INFO(0, image)
                ZEND_ARG_INFO(0, width)
                ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_insert_formula_arginfo, 0, 0, 3)
                ZEND_ARG_INFO(0, row)
                ZEND_ARG_INFO(0, column)
                ZEND_ARG_INFO(0, formula)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_auto_filter_arginfo, 0, 0, 1)
                ZEND_ARG_INFO(0, range)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_merge_cells_arginfo, 0, 0, 2)
                ZEND_ARG_INFO(0, range)
                ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_set_column_arginfo, 0, 0, 3)
                ZEND_ARG_INFO(0, format_handle)
                ZEND_ARG_INFO(0, range)
                ZEND_ARG_INFO(0, width)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xls_set_row_arginfo, 0, 0, 3)
                ZEND_ARG_INFO(0, format_handle)
                ZEND_ARG_INFO(0, range)
                ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()
/* }}} */

/** {{{ \Vtiful\Kernel\xls::__construct(array $config)
 */
PHP_METHOD(vtiful_xls, __construct)
{
    zval *config, *c_path;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ARRAY(config)
    ZEND_PARSE_PARAMETERS_END();

    if((c_path = zend_hash_str_find(Z_ARRVAL_P(config), ZEND_STRL(V_XLS_PAT))) == NULL)
    {
        zend_throw_exception(vtiful_exception_ce, "Lack of 'path' configuration", 110);
        return;
    }

    if(Z_TYPE_P(c_path) != IS_STRING)
    {
        zend_throw_exception(vtiful_exception_ce, "Configure 'path' must be a string type", 120);
        return;
    }

    add_property_zval(getThis(), V_XLS_COF, config);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::filename(string $fileName [, string $sheetName])
 */
PHP_METHOD(vtiful_xls, fileName)
{
    zval file_path, *dir_path = NULL;
    zend_string *zs_file_name = NULL, *zs_sheet_name = NULL;
    char *sheet_name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_STR(zs_file_name)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(zs_sheet_name)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    GET_CONFIG_PATH(dir_path, vtiful_xls_ce, return_value);

    xls_object *obj = Z_XLS_P(getThis());

    if(obj->ptr.workbook == NULL) {
        xls_file_path(zs_file_name, dir_path, &file_path);

        if(zs_sheet_name != NULL) {
            sheet_name = ZSTR_VAL(zs_sheet_name);
        }

        obj->ptr.workbook  = workbook_new(Z_STRVAL(file_path));
        obj->ptr.worksheet = workbook_add_worksheet(obj->ptr.workbook, sheet_name);

        add_property_zval(return_value, V_XLS_FIL, &file_path);

        zval_ptr_dtor(&file_path);
    }
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::addSheet(string $sheetName)
 */
PHP_METHOD(vtiful_xls, addSheet)
{
    zend_string *zs_sheet_name = NULL;
    char *sheet_name = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(zs_sheet_name)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    SHEET_LINE_INIT(obj)

    if(obj->ptr.workbook == NULL) {
        zend_throw_exception(vtiful_exception_ce, "Please create a file first, use the filename method", 130);
        return;
    }

    if(zs_sheet_name != NULL) {
        sheet_name = ZSTR_VAL(zs_sheet_name);
    }

    obj->ptr.worksheet = workbook_add_worksheet(obj->ptr.workbook, sheet_name);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::getSheetByName(string $sheetName)
 */
PHP_METHOD(vtiful_xls, getSheetByName)
{
    zend_string *zs_sheet_name = NULL;
    char *sheet_name = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(zs_sheet_name)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    SHEET_LINE_INIT(obj)

    if(obj->ptr.workbook == NULL) {
        zend_throw_exception(vtiful_exception_ce, "Please create a file first, use the filename method", 130);
        return;
    }

    if(zs_sheet_name != NULL) {
        sheet_name = ZSTR_VAL(zs_sheet_name);
    }

    obj->ptr.worksheet = workbook_get_worksheet_by_name(obj->ptr.workbook, sheet_name);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::setFreeze(int $row, int $column)
 */
PHP_METHOD(vtiful_xls, setFreeze)
{
    zend_long row, column;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    worksheet_freeze_panes(obj->ptr.worksheet, row, column);

}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::constMemory(string $fileName [, string $sheetName])
 */
PHP_METHOD(vtiful_xls, constMemory)
{
    zval file_path, *dir_path = NULL;
    zend_string *zs_file_name = NULL, *zs_sheet_name = NULL;
    char *sheet_name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_STR(zs_file_name)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(zs_sheet_name)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    GET_CONFIG_PATH(dir_path, vtiful_xls_ce, return_value);

    xls_object *obj = Z_XLS_P(getThis());

    if(obj->ptr.workbook == NULL) {
        xls_file_path(zs_file_name, dir_path, &file_path);

        lxw_workbook_options options = {.constant_memory = LXW_TRUE, .tmpdir = NULL};

        if(zs_sheet_name != NULL) {
            sheet_name = ZSTR_VAL(zs_sheet_name);
        }

        obj->ptr.workbook  = workbook_new_opt(Z_STRVAL(file_path), &options);
        obj->ptr.worksheet = workbook_add_worksheet(obj->ptr.workbook, sheet_name);

        add_property_zval(return_value, V_XLS_FIL, &file_path);

        zval_ptr_dtor(&file_path);
    }
}
/* }}} */


/** {{{ \Vtiful\Kernel\xls::header(array $header, array $format)
 */
PHP_METHOD(vtiful_xls, header)
{
    zval *header, *header_value;
    zend_long header_l_key;
    zval *format_handle = NULL;

    int argc  = ZEND_NUM_ARGS();

    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ARRAY(header)
            Z_PARAM_OPTIONAL         
            Z_PARAM_ARRAY(format_handle)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(header), header_l_key, header_value)
        if (argc == 1) {
            type_writer(header_value, 0, header_l_key, &obj->ptr, NULL);
        } else if (argc == 2) {
            zend_bool matched = LXW_FALSE;
            zend_long iRow;
            zend_string *sRow;
            zval *rowArray;
            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(format_handle), iRow, sRow, rowArray)
                if (iRow == 0) {
                    zend_long iCol;
                    zend_string *sCol;
                    zval *formatData;
                    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(rowArray), iCol, sCol, formatData)
                        if (iCol == header_l_key) {
                            type_writer2(header_value, 0, header_l_key, &obj->ptr, zval_get_format(formatData));
                            matched = LXW_TRUE;
                            break;
                        }
                    ZEND_HASH_FOREACH_END();
                }
                if (matched == LXW_TRUE) { break; }
            ZEND_HASH_FOREACH_END();
            if (matched == LXW_FALSE) {
                type_writer(header_value, 0, header_l_key, &obj->ptr, NULL);
            }
        }
        zval_ptr_dtor(header_value);
    ZEND_HASH_FOREACH_END();
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::data(array $data, array $format)
 */
PHP_METHOD(vtiful_xls, data)
{
    zval *data = NULL, *data_r_value = NULL;
    zval *format_handle = NULL;

    int argc  = ZEND_NUM_ARGS();

    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ARRAY(data)   
            Z_PARAM_OPTIONAL         
            Z_PARAM_ARRAY(format_handle)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), data_r_value)

        if(Z_TYPE_P(data_r_value) == IS_ARRAY) {

            SHEET_LINE_ADD(obj);

            zend_long current_row = SHEET_CURRENT_LINE(obj);

            ZEND_HASH_FOREACH_BUCKET(Z_ARRVAL_P(data_r_value), Bucket *bucket)

                zend_long current_col = bucket->h;

                if (argc == 1) {

                    type_writer(&bucket->val, current_row, current_col, &obj->ptr, NULL);

                } else if (argc == 2) {
                    zend_bool matched = LXW_FALSE;
                    zend_long iRow;
                    zend_string *sRow;
                    zval *rowArray;
                    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(format_handle), iRow, sRow, rowArray)
                        if ((current_row == 1 && iRow == current_row - 1) || iRow == current_row) {
                            zend_long iCol;
                            zend_string *sCol;
                            zval *formatData;
                            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(rowArray), iCol, sCol, formatData)
                                if (iCol == current_col) {
                                    type_writer2(&bucket->val, current_row, current_col, &obj->ptr, zval_get_format(formatData));
                                    matched = LXW_TRUE;
                                    break;
                                }
                            ZEND_HASH_FOREACH_END();
                        }
                        if (matched) { break; }
                    ZEND_HASH_FOREACH_END();
                    if (matched == LXW_FALSE) {
                        type_writer(&bucket->val, current_row, current_col, &obj->ptr, NULL);
                    }
                }
                zval_ptr_dtor(&bucket->val);
            ZEND_HASH_FOREACH_END();
        }

    ZEND_HASH_FOREACH_END();
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::output()
 */
PHP_METHOD(vtiful_xls, output)
{
    zval rv, *file_path;

    file_path = zend_read_property(vtiful_xls_ce, getThis(), ZEND_STRL(V_XLS_FIL), 0, &rv TSRMLS_DC);

    xls_object *obj = Z_XLS_P(getThis());

    workbook_file(&obj->ptr);

    add_property_null(getThis(), V_XLS_HANDLE);
    add_property_null(getThis(), V_XLS_PAT);

    ZVAL_COPY(return_value, file_path);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::getHandle()
 */
PHP_METHOD(vtiful_xls, getHandle)
{
    xls_object *obj = Z_XLS_P(getThis());

    RETURN_RES(zend_register_resource(&obj->ptr, le_xls_writer));
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::insertText(int $row, int $column, string|int|double $data)
 */
PHP_METHOD(vtiful_xls, insertText)
{
    zval *data;
    zend_long row, column;
    zend_string *format = NULL;

    ZEND_PARSE_PARAMETERS_START(3, 4)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
            Z_PARAM_ZVAL(data)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(format)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    type_writer(data, row, column, &obj->ptr, format);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::insertChart(int $row, int $column, resource $chartResource)
 */
PHP_METHOD(vtiful_xls, insertChart)
{
    zval      *chart_resource;
    zend_long row, column;

    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
            Z_PARAM_ZVAL(chart_resource)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    chart_writer(row, column, zval_get_chart(chart_resource), &obj->ptr);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::insertUrl(int $row, int $column, string $url)
 */
PHP_METHOD(vtiful_xls, insertUrl)
{
    zend_long row, column;
    zend_string *url = NULL;
    zval *format_handle = NULL;

    int argc = ZEND_NUM_ARGS();

    ZEND_PARSE_PARAMETERS_START(3, 4)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
            Z_PARAM_STR(url)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(format_handle)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    if (argc == 4) {
        url_writer(row, column, &obj->ptr, url, zval_get_format(format_handle));
    }

    if (argc == 3) {
        url_writer(row, column, &obj->ptr, url, NULL);
    }
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::insertImage(int $row, int $column, string $imagePath)
 */
PHP_METHOD(vtiful_xls, insertImage)
{
    zval *image;
    zend_long row, column;
    double width = 1, height = 1;

    ZEND_PARSE_PARAMETERS_START(3, 5)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
            Z_PARAM_ZVAL(image)
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(width)
            Z_PARAM_DOUBLE(height)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    image_writer(image, row, column, width, height,  &obj->ptr);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::insertImage(int $row, int $column, string $imagePath)
 */
PHP_METHOD(vtiful_xls, insertFormula)
{
    zval *formula;
    zend_long row, column;

    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_LONG(row)
            Z_PARAM_LONG(column)
            Z_PARAM_ZVAL(formula)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    formula_writer(formula, row, column, &obj->ptr);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::autoFilter(int $rowStart, int $rowEnd, int $columnStart, int $columnEnd)
 */
PHP_METHOD(vtiful_xls, autoFilter)
{
    zend_string *range;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(range)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    auto_filter(range, &obj->ptr);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::mergeCells(string $range, string $data)
 */
PHP_METHOD(vtiful_xls, mergeCells)
{
    zend_string *range, *data;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_STR(range)
            Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    merge_cells(range, data, &obj->ptr);
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::setColumn(resource $format, string $range [, int $width])
 */
PHP_METHOD(vtiful_xls, setColumn)
{
    zval *format_handle;
    zend_string *range;

    double width = 0;
    int    argc  = ZEND_NUM_ARGS();

    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_STR(range)
            Z_PARAM_DOUBLE(width)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(format_handle)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    if (argc == 3) {
        set_column(range, width, &obj->ptr, zval_get_format(format_handle));
    }

    if (argc == 2) {
        set_column(range, width, &obj->ptr, NULL);
    }
}
/* }}} */

/** {{{ \Vtiful\Kernel\xls::setRow(resource $format, string $range [, int $heitght])
 */
PHP_METHOD(vtiful_xls, setRow)
{
    zval *format_handle;
    zend_string *range;

    double height = 0;
    int    argc  = ZEND_NUM_ARGS();

    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_STR(range)
            Z_PARAM_DOUBLE(height)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(format_handle)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_COPY(return_value, getThis());

    xls_object *obj = Z_XLS_P(getThis());

    if (argc == 3) {
        set_row(range, height, &obj->ptr, zval_get_format(format_handle));
    }

    if (argc == 2) {
        set_row(range, height, &obj->ptr, NULL);
    }
}
/* }}} */

/** {{{ xls_methods
*/
zend_function_entry xls_methods[] = {
        PHP_ME(vtiful_xls, __construct,         xls_construct_arginfo,      ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, fileName,            xls_file_name_arginfo,      ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, addSheet,            xls_file_add_sheet,         ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, getSheetByName,      xls_get_sheet_name,         ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, setFreeze,           xls_freeze_arginfo,         ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, constMemory,         xls_const_memory_arginfo,   ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, header,              xls_header_arginfo,         ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, data,                xls_data_arginfo,           ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, output,              NULL,                       ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, getHandle,           NULL,                       ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, autoFilter,          xls_auto_filter_arginfo,    ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, insertText,          xls_insert_text_arginfo,    ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, insertChart,         xls_insert_chart_arginfo,   ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, insertUrl,           xls_insert_url_arginfo,     ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, insertImage,         xls_insert_image_arginfo,   ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, insertFormula,       xls_insert_formula_arginfo, ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, mergeCells,          xls_merge_cells_arginfo,    ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, setColumn,           xls_set_column_arginfo,     ZEND_ACC_PUBLIC)
        PHP_ME(vtiful_xls, setRow,              xls_set_row_arginfo,        ZEND_ACC_PUBLIC)
        PHP_FE_END
};
/* }}} */

/** {{{ VTIFUL_STARTUP_FUNCTION
*/
VTIFUL_STARTUP_FUNCTION(excel) {
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Vtiful\\Kernel", "Excel", xls_methods);
    ce.create_object = vtiful_xls_objects_new;
    vtiful_xls_ce = zend_register_internal_class(&ce);

    memcpy(&vtiful_xls_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    vtiful_xls_handlers.offset   = XtOffsetOf(xls_object, zo);
    vtiful_xls_handlers.free_obj = vtiful_xls_objects_free;

    REGISTER_CLASS_PROPERTY_NULL(vtiful_xls_ce, V_XLS_COF, ZEND_ACC_PRIVATE);
    REGISTER_CLASS_PROPERTY_NULL(vtiful_xls_ce, V_XLS_FIL, ZEND_ACC_PRIVATE);

    return SUCCESS;
}
/* }}} */



