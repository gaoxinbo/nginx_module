#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_config.h>

static char* ngx_http_backtrace(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); 
static void* ngx_http_backtrace_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_backtrace_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
static ngx_int_t ngx_http_backtrace_handler(ngx_http_request_t *r);

typedef struct{
  ngx_int_t depth;
} ngx_http_backtrace_conf_t;

static ngx_command_t ngx_http_backtrace_command[] = {
  {
    ngx_string("backtrace"),
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_backtrace,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL,
  },
  {
    ngx_string("set_depth"),
    NGX_HTTP_MAIN_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_backtrace_conf_t,depth),
    NULL,
  },
  ngx_null_command
};

static ngx_http_module_t ngx_http_backtrace_module_ctx = {
  NULL,  /* preconfiguration */
  NULL,  /* postconfiguration */
  NULL,  /* create main configuration */
  NULL,  /* init main configuration */
  NULL,  /* create server configuration */
  NULL,  /* merge server configuration */
  ngx_http_backtrace_create_loc_conf,  /* create location configration */
  ngx_http_backtrace_merge_loc_conf,  /* merge location configration */
};

ngx_module_t ngx_http_backtrace_module = {
  NGX_MODULE_V1,
  &ngx_http_backtrace_module_ctx,
  ngx_http_backtrace_command,
  NGX_HTTP_MODULE,
  NULL,        /* init master */
  NULL,        /* init module */
  NULL,        /* init process */
  NULL,        /* init thread */
  NULL,        /* exit thread */
  NULL,        /* exit process */
  NULL,        /* exit master */
  NGX_MODULE_V1_PADDING
};


static void * ngx_http_backtrace_create_loc_conf(ngx_conf_t *cf){
  ngx_http_backtrace_conf_t *conf;
  conf = ngx_pcalloc(cf->pool,sizeof(ngx_http_backtrace_conf_t));
  if(conf==NULL){
    return NGX_CONF_ERROR;
  }
  conf->depth = 10;
  return conf;
}

static char * ngx_http_backtrace_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child){
  ngx_http_backtrace_conf_t *prev = parent;
  ngx_http_backtrace_conf_t *conf = child;
  ngx_conf_merge_value(conf->depth, prev->depth, 10);
  if(conf->depth<1){
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "backtrace depth must be bigger than 1");
    return NGX_CONF_ERROR;
  }
  return NGX_CONF_OK;
}

static char * ngx_http_backtrace(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){
  ngx_http_core_loc_conf_t *current;
  current = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
  current->handler = ngx_http_backtrace_handler;
  return NGX_CONF_OK;
}

static ngx_int_t ngx_http_backtrace_handler(ngx_http_request_t *r){
  ngx_int_t    rc;
  ngx_buf_t   *b;
  ngx_chain_t  out;
  ngx_http_backtrace_conf_t* my_conf;
  u_char ngx_hello_string[1024] = {0};
  ngx_uint_t content_length = 0;

  ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_hello_handler is called!");

  my_conf = ngx_http_get_module_loc_conf(r, ngx_http_backtrace_module);
  ngx_sprintf(ngx_hello_string, "Depth:%d", my_conf->depth);
  ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello_string:%s", ngx_hello_string);
  content_length = ngx_strlen(ngx_hello_string);

  /* we response to 'GET' and 'HEAD' requests only */
  if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
    return NGX_HTTP_NOT_ALLOWED;
  }

  /* discard request body, since we don't need it here */
  rc = ngx_http_discard_request_body(r);

  if (rc != NGX_OK) {
    return rc;
  }

  /* set the 'Content-type' header */
  /*
     r->headers_out.content_type_len = sizeof("text/html") - 1;
     r->headers_out.content_type.len = sizeof("text/html") - 1;
     r->headers_out.content_type.data = (u_char *)"text/html";*/
  ngx_str_set(&r->headers_out.content_type, "text/html");


  /* send the header only, if the request type is http 'HEAD' */
  if (r->method == NGX_HTTP_HEAD) {
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;

    return ngx_http_send_header(r);
  }

  /* allocate a buffer for your response body */
  b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
  if (b == NULL) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  /* attach this buffer to the buffer chain */
  out.buf = b;
  out.next = NULL;

  /* adjust the pointers of the buffer */
  b->pos = ngx_hello_string;
  b->last = ngx_hello_string + content_length;
  b->memory = 1;    /* this buffer is in memory */
  b->last_buf = 1;  /* this is the last buffer in the buffer chain */

  /* set the status line */
  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = content_length;

  /* send the headers of your response */
  rc = ngx_http_send_header(r);

  if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    return rc;
  }

  /* send the buffer chain of your response */
  return ngx_http_output_filter(r, &out);
}

