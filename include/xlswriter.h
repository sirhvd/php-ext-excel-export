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

#ifndef PHP_XLS_WRITER_INCLUDE_H
#define PHP_XLS_WRITER_INCLUDE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#include "zend_exceptions.h"
#include "zend.h"
#include "zend_API.h"
#include "php.h"

#include "xlsxwriter.h"
#include "xlsxwriter/packager.h"
#include "xlsxwriter/format.h"

#include "php_xlswriter.h"
#include "excel.h"
#include "exception.h"
#include "format.h"
#include "chart.h"

typedef struct {
    lxw_workbook  *workbook;
    lxw_worksheet *worksheet;
} xls_resource_t;

typedef struct {
    lxw_format  *format;
} xls_resource_format_t;

typedef struct {
    lxw_chart *chart;
    lxw_chart_series *series;
} xls_resource_chart_t;

typedef struct _vtiful_xls_object {
    xls_resource_t ptr;
    zend_long      line;
    zend_object    zo;
} xls_object;

typedef struct _vtiful_format_object {
    xls_resource_format_t ptr;
    zend_object zo;
} format_object;

typedef struct _vtiful_chart_object {
    xls_resource_chart_t ptr;
    zend_object zo;
} chart_object;

static inline xls_object *php_vtiful_xls_fetch_object(zend_object *obj) {
    return (xls_object *)((char *)(obj) - XtOffsetOf(xls_object, zo));
}

static inline format_object *php_vtiful_format_fetch_object(zend_object *obj) {
    return (format_object *)((char *)(obj) - XtOffsetOf(format_object, zo));
}

static inline chart_object *php_vtiful_chart_fetch_object(zend_object *obj) {
    return (chart_object *)((char *)(obj) - XtOffsetOf(chart_object, zo));
}

#define REGISTER_CLASS_CONST_LONG(class_name, const_name, value) \
    zend_declare_class_constant_long(class_name, const_name, sizeof(const_name)-1, (zend_long)value);

#define REGISTER_CLASS_PROPERTY_NULL(class_name, property_name, acc) \
    zend_declare_property_null(class_name, ZEND_STRL(property_name), acc);

#define Z_XLS_P(zv)    php_vtiful_xls_fetch_object(Z_OBJ_P(zv));
#define Z_CHART_P(zv)  php_vtiful_chart_fetch_object(Z_OBJ_P(zv));
#define Z_FORMAT_P(zv) php_vtiful_format_fetch_object(Z_OBJ_P(zv));

#define ROW(range) \
    lxw_name_to_row(range)

#define ROWS(range) \
    lxw_name_to_row(range), lxw_name_to_row_2(range)

#define SHEET_LINE_INIT(obj_p) \
    obj_p->line = 0;

#define SHEET_LINE_ADD(obj_p) \
    ++obj_p->line;

#define SHEET_CURRENT_LINE(obj_p) obj_p->line

lxw_format           * zval_get_format(zval *handle);
xls_resource_t       * zval_get_resource(zval *handle);
xls_resource_chart_t *zval_get_chart(zval *resource);

STATIC lxw_error _store_defined_name(lxw_workbook *self, const char *name, const char *app_name, const char *formula, int16_t index, uint8_t hidden);

STATIC void _prepare_defined_names(lxw_workbook *self);
STATIC void _prepare_drawings(lxw_workbook *self);
STATIC void _add_chart_cache_data(lxw_workbook *self);
STATIC int  _compare_defined_names(lxw_defined_name *a, lxw_defined_name *b);
STATIC void _populate_range(lxw_workbook *self, lxw_series_range *range);
STATIC void _populate_range_dimensions(lxw_workbook *self, lxw_series_range *range);

void type_writer(zval *value, zend_long row, zend_long columns, xls_resource_t *res, zend_string *format);
void type_writer2(zval *value, zend_long row, zend_long columns, xls_resource_t *res, lxw_format *format);
void chart_writer(zend_long row, zend_long columns, xls_resource_chart_t *chart_resource, xls_resource_t *res);
void url_writer(zend_long row, zend_long columns, xls_resource_t *res, zend_string *url, lxw_format *format);
void image_writer(zval *value, zend_long row, zend_long columns, double width, double height, xls_resource_t *res);
void formula_writer(zval *value, zend_long row, zend_long columns, xls_resource_t *res);
void auto_filter(zend_string *range, xls_resource_t *res);
void merge_cells(zend_string *range, zend_string *value, xls_resource_t *res);
void set_column(zend_string *range, double width, xls_resource_t *res, lxw_format *format);
void set_row(zend_string *range, double height, xls_resource_t *res, lxw_format *format);
void worksheet_set_rows(lxw_row_t start, lxw_row_t end, double height, xls_resource_t *res, lxw_format *format);
lxw_error workbook_file(xls_resource_t *self);

void xls_file_path(zend_string *file_name, zval *dir_path, zval *file_path);

#endif
