#ifndef _INCLUDE_NXS_CHAT_SRV_D_RDMN_ISSUES_H
#define _INCLUDE_NXS_CHAT_SRV_D_RDMN_ISSUES_H

// clang-format off

/* Structs declarations */



/* Prototypes */

nxs_chat_srv_err_t			nxs_chat_srv_d_rdmn_issues_add_comment		(size_t issue_id, size_t assigned_to_id, nxs_string_t *note, nxs_bool_t private_notes, size_t status_id, nxs_array_t *uploads, nxs_array_t *custom_fields, nxs_string_t *user_api_key, nxs_http_code_t *http_code, nxs_buf_t *out_buf);
nxs_chat_srv_err_t			nxs_chat_srv_d_rdmn_issues_create		(size_t project_id, size_t priority_id, nxs_string_t *subject, nxs_string_t *description, nxs_bool_t is_private, nxs_array_t *uploads, nxs_string_t *user_api_key, nxs_http_code_t *http_code, nxs_buf_t *out_buf);

nxs_chat_srv_err_t			nxs_chat_srv_d_rdmn_issues_get_query		(size_t issue_query_id, nxs_string_t *user_api_key, size_t offset, size_t limit, nxs_http_code_t *http_code, nxs_buf_t *out_buf);
nxs_chat_srv_err_t			nxs_chat_srv_d_rdmn_issues_get_issue		(size_t issue_id, nxs_string_t *user_api_key, nxs_http_code_t *http_code, nxs_buf_t *out_buf);

void					nxs_chat_srv_d_rdmn_issues_cf_init		(nxs_array_t *custom_fields);
void					nxs_chat_srv_d_rdmn_issues_cf_free		(nxs_array_t *custom_fields);
void					nxs_chat_srv_d_rdmn_issues_cf_add		(nxs_array_t *custom_fields, size_t id, nxs_string_t *value);

void					nxs_chat_srv_d_rdmn_issues_uploads_init		(nxs_array_t *uploads);
void					nxs_chat_srv_d_rdmn_issues_uploads_free		(nxs_array_t *uploads);
void					nxs_chat_srv_d_rdmn_issues_uploads_add		(nxs_array_t *uploads, nxs_string_t *token, nxs_string_t *file_name, nxs_string_t *file_path, nxs_string_t *content_type);
nxs_chat_srv_err_t			nxs_chat_srv_d_rdmn_issues_uploads_push		(nxs_string_t *user_api_key, nxs_string_t *file_path, nxs_http_code_t *http_code, nxs_buf_t *out_buf);

#endif /* _INCLUDE_NXS_CHAT_SRV_D_RDMN_ISSUES_H */
