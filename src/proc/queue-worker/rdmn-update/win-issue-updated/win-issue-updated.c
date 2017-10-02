#include <nxs-core/nxs-core.h>

// clang-format off

/* Project core include */
#include <nxs-chat-srv-core.h>
#include <nxs-chat-srv-meta.h>
#include <nxs-chat-srv-collections.h>
#include <nxs-chat-srv-units.h>

#include <proc/queue-worker/rdmn-update/win-issue-updated/win-issue-updated.h>
#include <proc/queue-worker/rdmn-update/win-issue-updated/ctx/win-issue-updated-ctx.h>
#include <proc/queue-worker/rdmn-update/win-issue-updated/win-issue-updated-subproc.h>

/* Definitions */



/* Project globals */
extern		nxs_process_t			process;
extern		nxs_chat_srv_cfg_t		nxs_chat_srv_cfg;

/* Module typedefs */



/* Module declarations */

typedef enum
{
	JOURNAL_PROPERTY_NONE,
	JOURNAL_PROPERTY_IS_PRIVATE,
	JOURNAL_PROPERTY_STATUS_ID,
	JOURNAL_PROPERTY_PRIORITY_ID,
	JOURNAL_PROPERTY_ASSIGNED_TO_ID,
} journal_property_t;

typedef struct
{
	nxs_string_t		name;
	journal_property_t	property;
} journal_properties_t;

/* Module internal (static) functions prototypes */

// clang-format on

static journal_property_t journal_property_get(nxs_string_t *property_name);

// clang-format off

/* Module initializations */

static journal_properties_t journal_properties[] =
{
	{nxs_string("is_private"),		JOURNAL_PROPERTY_IS_PRIVATE},
	{nxs_string("status_id"),		JOURNAL_PROPERTY_STATUS_ID},
	{nxs_string("priority_id"),		JOURNAL_PROPERTY_PRIORITY_ID},
	{nxs_string("assigned_to_id"),		JOURNAL_PROPERTY_ASSIGNED_TO_ID},

	{NXS_STRING_NULL_STR,			JOURNAL_PROPERTY_NONE}
};

static u_char		_s_private_message[]	= {NXS_CHAT_SRV_UTF8_PRIVATE_MESSAGE};

/* Module global functions */

// clang-format on

nxs_chat_srv_err_t nxs_chat_srv_p_queue_worker_rdmn_update_win_issue_updated_runtime(nxs_chat_srv_m_rdmn_update_t * update,
                                                                                     size_t                         tlgrm_userid,
                                                                                     nxs_chat_srv_m_rdmn_journal_t *journal)
{
	nxs_chat_srv_u_tlgrm_sendmessage_t *tlgrm_sendmessage_ctx;
	nxs_chat_srv_m_rdmn_detail_t *      d;
	nxs_string_t message, properties, property_is_private, property_status, property_priority, property_assigned_to, notes_fmt,
	        project_fmt, subject_fmt, user_fmt, status_fmt, priority_fmt, assigned_to_fmt, *s;
	nxs_buf_t *                    out_buf;
	nxs_array_t                    m_chunks;
	nxs_bool_t                     response_status;
	nxs_chat_srv_m_tlgrm_message_t tlgrm_message;
	nxs_chat_srv_u_db_issues_t *   db_issue_ctx;
	nxs_bool_t                     use_property;
	size_t                         i;

	if(update == NULL || journal == NULL) {

		return NXS_CHAT_SRV_E_PTR;
	}

	nxs_chat_srv_err_t rc;

	rc = NXS_CHAT_SRV_E_OK;

	nxs_string_init(&message);
	nxs_string_init_empty(&properties);
	nxs_string_init_empty(&property_is_private);
	nxs_string_init_empty(&property_status);
	nxs_string_init_empty(&property_priority);
	nxs_string_init_empty(&property_assigned_to);
	nxs_string_init_empty(&notes_fmt);
	nxs_string_init_empty(&project_fmt);
	nxs_string_init_empty(&subject_fmt);
	nxs_string_init_empty(&user_fmt);
	nxs_string_init_empty(&status_fmt);
	nxs_string_init_empty(&priority_fmt);
	nxs_string_init_empty(&assigned_to_fmt);

	nxs_array_init2(&m_chunks, nxs_string_t);

	tlgrm_sendmessage_ctx = nxs_chat_srv_u_tlgrm_sendmessage_init();
	db_issue_ctx          = nxs_chat_srv_u_db_issues_init();

	nxs_chat_srv_c_tlgrm_message_init(&tlgrm_message);

	/*
	 * Prepare property block content
	 */

	use_property = NXS_NO;

	for(i = 0; i < nxs_array_count(&journal->details); i++) {

		d = nxs_array_get(&journal->details, i);

		switch(journal_property_get(&d->name)) {

			case JOURNAL_PROPERTY_IS_PRIVATE:

				use_property = NXS_YES;

				if(update->data.issue.is_private == NXS_YES) {

					nxs_string_printf(&property_is_private, NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_IS_PRIVATE_YES);
				}
				else {

					nxs_string_printf(&property_is_private, NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_IS_PRIVATE_NO);
				}

				break;

			case JOURNAL_PROPERTY_STATUS_ID:

				use_property = NXS_YES;

				nxs_chat_srv_c_tlgrm_format_escape_html(&status_fmt, &update->data.issue.status.name);

				nxs_string_printf(&property_status, NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_STATUS, &status_fmt);

				break;

			case JOURNAL_PROPERTY_PRIORITY_ID:

				use_property = NXS_YES;

				nxs_chat_srv_c_tlgrm_format_escape_html(&priority_fmt, &update->data.issue.priority.name);

				nxs_string_printf(&property_priority, NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_PRIORITY, &priority_fmt);

				break;

			case JOURNAL_PROPERTY_ASSIGNED_TO_ID:

				use_property = NXS_YES;

				nxs_chat_srv_c_tlgrm_format_escape_html(&assigned_to_fmt, &update->data.issue.assigned_to.name);

				nxs_string_printf(
				        &property_assigned_to, NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_ASSIGNED_TO, &assigned_to_fmt);

				break;

			default:

				break;
		}
	}

	if(use_property == NXS_YES) {

		nxs_string_printf(
		        &properties, "%r%r%r%r\n", &property_is_private, &property_status, &property_priority, &property_assigned_to);
	}

	nxs_chat_srv_c_tlgrm_format_escape_html(&project_fmt, &update->data.issue.project.name);
	nxs_chat_srv_c_tlgrm_format_escape_html(&subject_fmt, &update->data.issue.subject);

	if(nxs_string_len(&journal->notes) > 0) {

		nxs_chat_srv_c_tlgrm_format_escape_html(&notes_fmt, &journal->notes);
		nxs_chat_srv_c_tlgrm_format_escape_html(&user_fmt, &journal->user.name);

		nxs_string_printf(&message,
		                  NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED,
		                  journal->private_notes == NXS_YES ? (char *)_s_private_message : "",
		                  &nxs_chat_srv_cfg.rdmn.host,
		                  update->data.issue.id,
		                  &project_fmt,
		                  update->data.issue.id,
		                  &subject_fmt,
		                  &properties,
		                  &user_fmt);

		nxs_chat_srv_c_tlgrm_make_message_chunks(&message, &notes_fmt, &m_chunks);
	}
	else {

		if(use_property == NXS_YES) {

			nxs_string_printf(&message,
			                  NXS_CHAT_SRV_RDMN_MESSAGE_ISSUE_UPDATED_NO_MESSAGE,
			                  journal->private_notes == NXS_YES ? (char *)_s_private_message : "",
			                  &nxs_chat_srv_cfg.rdmn.host,
			                  update->data.issue.id,
			                  &project_fmt,
			                  update->data.issue.id,
			                  &subject_fmt,
			                  &properties);

			nxs_chat_srv_c_tlgrm_make_message_chunks(&message, NULL, &m_chunks);
		}
		else {

			nxs_error(rc, NXS_CHAT_SRV_E_OK, error);
		}
	}

	/* create new comment: send message chunks */
	for(i = 0; i < nxs_array_count(&m_chunks); i++) {

		s = nxs_array_get(&m_chunks, i);

		nxs_chat_srv_u_tlgrm_sendmessage_add(tlgrm_sendmessage_ctx, tlgrm_userid, s, NXS_CHAT_SRV_M_TLGRM_PARSE_MODE_TYPE_HTML);

		nxs_chat_srv_u_tlgrm_sendmessage_disable_web_page_preview(tlgrm_sendmessage_ctx);

		if(nxs_chat_srv_u_tlgrm_sendmessage_push(tlgrm_sendmessage_ctx) != NXS_CHAT_SRV_E_OK) {

			nxs_error(rc, NXS_CHAT_SRV_E_ERR, error);
		}

		out_buf = nxs_chat_srv_u_tlgrm_sendmessage_get_response_buf(tlgrm_sendmessage_ctx);

		if(nxs_chat_srv_c_tlgrm_message_result_pull_json(&tlgrm_message, &response_status, out_buf) == NXS_CHAT_SRV_E_OK) {

			nxs_chat_srv_u_db_issues_set(db_issue_ctx, tlgrm_userid, tlgrm_message.message_id, update->data.issue.id);
		}
	}

error:

	tlgrm_sendmessage_ctx = nxs_chat_srv_u_tlgrm_sendmessage_free(tlgrm_sendmessage_ctx);
	db_issue_ctx          = nxs_chat_srv_u_db_issues_free(db_issue_ctx);

	nxs_chat_srv_c_tlgrm_message_free(&tlgrm_message);

	for(i = 0; i < nxs_array_count(&m_chunks); i++) {

		s = nxs_array_get(&m_chunks, i);

		nxs_string_free(s);
	}

	nxs_array_free(&m_chunks);

	nxs_string_free(&message);
	nxs_string_free(&properties);
	nxs_string_free(&property_is_private);
	nxs_string_free(&property_status);
	nxs_string_free(&property_priority);
	nxs_string_free(&property_assigned_to);
	nxs_string_free(&notes_fmt);
	nxs_string_free(&project_fmt);
	nxs_string_free(&subject_fmt);
	nxs_string_free(&user_fmt);
	nxs_string_free(&status_fmt);
	nxs_string_free(&priority_fmt);
	nxs_string_free(&assigned_to_fmt);

	return rc;
}

/* Module internal (static) functions */

static journal_property_t journal_property_get(nxs_string_t *property_name)
{
	size_t i;

	for(i = 0; journal_properties[i].property != JOURNAL_PROPERTY_NONE; i++) {

		if(nxs_string_cmp(&journal_properties[i].name, 0, property_name, 0) == NXS_YES) {

			return journal_properties[i].property;
		}
	}

	return JOURNAL_PROPERTY_NONE;
}
