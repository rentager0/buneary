#ifndef _INCLUDE_MISTRAL_PRINT_H_
#define _INCLUDE_MISTRAL_PRINT_H_


#include "mistral/basic.h"
#include "mistral/loader.h"
#include "mistral/payload.h"


void print_title(const char *format, ...);

void print_subtitle(const char *format, ...);

void print_entry(const char *format, ...);

void print_subentry(const char *format, ...);


void print_loader_scheme(const struct loader_scheme *scheme);

void print_payload_scheme(const struct payload_scheme *scheme);

void print_basic_scheme(const struct basic_scheme *scheme);

void print_pagetable(const struct basic_scheme *scheme);


#endif
