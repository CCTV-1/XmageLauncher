#pragma once
#ifndef FILEUTILITIES_H
#define FILEUTILITIES_H

#include <future>

#include <glibmm.h>

#include "commontype.h"

std::shared_future<void> unzip_client( Glib::ustring client_zip_name , Glib::ustring unzip_path );

#endif