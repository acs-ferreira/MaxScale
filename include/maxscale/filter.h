#pragma once
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 * @file filter.h -  The filter interface mechanisms
 *
 * Revision History
 *
 * Date         Who                     Description
 * 27/05/2014   Mark Riddoch            Initial implementation
 *
 */

#include <maxscale/cdefs.h>
#include <maxscale/routing.h>
#include <maxscale/dcb.h>
#include <maxscale/session.h>
#include <maxscale/buffer.h>
#include <stdint.h>

MXS_BEGIN_DECLS

/**
 * The FILTER handle points to module specific data, so the best we can do
 * is to make it a void * externally.
 */
typedef void *FILTER;

/**
 * The structure used to pass name, value pairs to the filter instances
 */
typedef struct
{
    char    *name;          /**< Name of the parameter */
    char    *value;         /**< Value of the parameter */
} FILTER_PARAMETER;

/**
 * @verbatim
 * The "module object" structure for a query router module
 *
 * The entry points are:
 *      createInstance          Called by the service to create a new
 *                              instance of the filter
 *      newSession              Called to create a new user session
 *                              within the filter
 *      closeSession            Called when a session is closed
 *      freeSession             Called when a session is freed
 *      setDownstream           Sets the downstream component of the
 *                              filter pipline
 *      routeQuery              Called on each query that requires
 *                              routing
 *      clientReply             Called for each reply packet
 *      diagnostics             Called to force the filter to print
 *                              diagnostic output
 *
 * @endverbatim
 *
 * @see load_module
 */
typedef struct filter_object
{
    FILTER *(*createInstance)(const char *name,
                              char **options,
                              FILTER_PARAMETER **params);
    void   *(*newSession)(FILTER *instance, SESSION *session);
    void   (*closeSession)(FILTER *instance, void *fsession);
    void   (*freeSession)(FILTER *instance, void *fsession);
    void   (*setDownstream)(FILTER *instance, void *fsession, DOWNSTREAM *downstream);
    void   (*setUpstream)(FILTER *instance, void *fsession, UPSTREAM *downstream);
    int    (*routeQuery)(FILTER *instance, void *fsession, GWBUF *queue);
    int    (*clientReply)(FILTER *instance, void *fsession, GWBUF *queue);
    void   (*diagnostics)(FILTER *instance, void *fsession, DCB *dcb);
    uint64_t (*getCapabilities)(void);
} FILTER_OBJECT;

/**
 * The filter API version. If the FILTER_OBJECT structure or the filter API
 * is changed these values must be updated in line with the rules in the
 * file modinfo.h.
 */
#define FILTER_VERSION  {2, 2, 0}
/**
 * The definition of a filter from the configuration file.
 * This is basically the link between a plugin to load and the
 * optons to pass to that plugin.
 */
typedef struct filter_def
{
    char *name;                    /**< The Filter name */
    char *module;                  /**< The module to load */
    char **options;                /**< The options set for this filter */
    FILTER_PARAMETER **parameters; /**< The filter parameters */
    FILTER filter;                 /**< The runtime filter */
    FILTER_OBJECT *obj;            /**< The "MODULE_OBJECT" for the filter */
    SPINLOCK spin;                 /**< Spinlock to protect the filter definition */
    struct filter_def *next;       /**< Next filter in the chain of all filters */
} FILTER_DEF;

FILTER_DEF *filter_alloc(char *, char *);
void filter_free(FILTER_DEF *);
bool filter_load(FILTER_DEF* filter);
FILTER_DEF *filter_find(char *);
void filterAddOption(FILTER_DEF *, char *);
void filterAddParameter(FILTER_DEF *, char *, char *);
DOWNSTREAM *filterApply(FILTER_DEF *, SESSION *, DOWNSTREAM *);
UPSTREAM *filterUpstream(FILTER_DEF *, void *, UPSTREAM *);
int filter_standard_parameter(char *);
void dprintAllFilters(DCB *);
void dprintFilter(DCB *, FILTER_DEF *);
void dListFilters(DCB *);
int filter_count_filters();

/**
 * Specifies capabilities specific for filters. Common capabilities
 * are defined by @c routing_capability_t.
 *
 * @see routing_capability_t
 *
 * @note The values of the capabilities here *must* be between 0x000100000000
 *       and 0x800000000000, that is, bits 32 to 47.
 */

/*
typedef enum filter_capability
{
} filter_capability_t;
*/

MXS_END_DECLS
