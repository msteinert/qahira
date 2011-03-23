
#ifndef __qahira_marshal_MARSHAL_H__
#define __qahira_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* POINTER:INT,INT,INT (../../qahira/marshal.list:1) */
extern void qahira_marshal_POINTER__INT_INT_INT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* POINTER:POINTER (../../qahira/marshal.list:2) */
extern void qahira_marshal_POINTER__POINTER (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* INT:POINTER (../../qahira/marshal.list:3) */
extern void qahira_marshal_INT__POINTER (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

/* VOID:POINTER (../../qahira/marshal.list:4) */
#define qahira_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* OBJECT:STRING (../../qahira/marshal.list:5) */
extern void qahira_marshal_OBJECT__STRING (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

G_END_DECLS

#endif /* __qahira_marshal_MARSHAL_H__ */

