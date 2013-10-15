Limitation with apache/php upload file size

Edit php.ini:

; Maximum size of POST data that PHP will accept.
post_max_size = 80M

; Maximum allowed size for uploaded files.
upload_max_filesize = 80M
