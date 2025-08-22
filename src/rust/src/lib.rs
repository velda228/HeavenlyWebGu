use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::ptr;

mod html_parser;
mod css_parser;
mod network;
mod security;

pub use html_parser::HtmlParser;
pub use css_parser::CssParser;
pub use network::NetworkManager;
pub use security::SecurityManager;

// FFI интерфейсы для C++

#[no_mangle]
pub extern "C" fn html_parse_new() -> *mut HtmlParser {
    let parser = HtmlParser::new();
    Box::into_raw(Box::new(parser))
}

#[no_mangle]
pub extern "C" fn html_parse_free(parser: *mut HtmlParser) {
    if !parser.is_null() {
        unsafe {
            let _ = Box::from_raw(parser);
        }
    }
}

#[no_mangle]
pub extern "C" fn html_parse_string(
    parser: *mut HtmlParser,
    html: *const c_char,
) -> *mut c_char {
    if parser.is_null() || html.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let parser_ref = &*parser;
        let html_str = CStr::from_ptr(html).to_string_lossy();
        
        match (*parser).parse(&html_str) {
            Ok(result) => {
                let c_string = CString::new(result).unwrap();
                c_string.into_raw()
            }
            Err(_) => ptr::null_mut(),
        }
    }
}

#[no_mangle]
pub extern "C" fn network_fetch_url(url: *const c_char) -> *mut c_char {
    if url.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let url_str = CStr::from_ptr(url).to_string_lossy();
        
        // Синхронный вызов (в реальности лучше использовать async)
        let rt = tokio::runtime::Runtime::new().unwrap();
        let result = rt.block_on(async move {
            let client = reqwest::Client::new();
            let resp = client.get(&*url_str).send().await?;
            let text = resp.text().await?;
            Ok::<String, Box<dyn std::error::Error>>(text)
        });

        match result {
            Ok(text) => {
                let c_string = CString::new(text).unwrap();
                c_string.into_raw()
            }
            Err(_) => ptr::null_mut(),
        }
    }
}

#[no_mangle]
pub extern "C" fn string_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe {
            let _ = CString::from_raw(ptr);
        }
    }
}
