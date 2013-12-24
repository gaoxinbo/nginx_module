#!/bin/bash


function help(){
  echo "usage: $0 <module_name>"
}

if [ $# == "0" ];then
  help
  exit
    help
    exit
fi

module=$1

if [ ! -d $module ];then
  mkdir $module
fi


cat << END > $module/config
ngx_addon_name=ngx_http_${module}
HTTP_MODULES="\$HTTP_MODULES ngx_http_${module}_module"
NGX_ADDON_SRCS="\$NGX_ADDON_SRCS \$ngx_addon_dir/ngx_http_${module}.c"
END


cat << END > $module/ngx_http_${module}.c
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_config.h>

typedef struct{
} ngx_http_${module}_conf_t;

static ngx_command_t ngx_http_${module}_command[] = {
    ngx_null_command
};

static ngx_http_module_t ngx_http_${module}_module_ctx = {
  NULL,  /* preconfiguration */
  NULL,  /* postconfiguration */
  NULL,  /* create main configuration */
  NULL,  /* init main configuration */
  NULL,  /* create server configuration */
  NULL,  /* merge server configuration */
  NULL,  /* create location configration */
  NULL,  /* merge location configration */
};

ngx_module_t ngx_http_backtrace_module = {
  NGX_MODULE_V1,
  &ngx_http_${module}_module_ctx,
  ngx_http_${module}_command,
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

END
