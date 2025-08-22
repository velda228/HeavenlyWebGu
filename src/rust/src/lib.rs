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

// Новая функция: возвращает количество элементов
#[no_mangle]
pub extern "C" fn html_get_element_count(parser: *mut HtmlParser) -> usize {
    if parser.is_null() {
        return 0;
    }
    unsafe {
        (*parser).get_element_count()
    }
}

// Новая функция: получает имя тега по индексу
#[no_mangle]
pub extern "C" fn html_get_element_tag_name(
    parser: *mut HtmlParser, 
    index: usize
) -> *mut c_char {
    if parser.is_null() {
        return ptr::null_mut();
    }
    
    unsafe {
        match (*parser).get_element_tag_name(index) {
            Some(tag_name) => {
                let c_string = CString::new(tag_name.as_str()).unwrap();
                c_string.into_raw()
            }
            None => ptr::null_mut(),
        }
    }
}

// Новая функция: получает текст элемента по индексу
#[no_mangle]
pub extern "C" fn html_get_element_text(
    parser: *mut HtmlParser, 
    index: usize
) -> *mut c_char {
    if parser.is_null() {
        return ptr::null_mut();
    }
    
    unsafe {
        match (*parser).get_element_text(index) {
            Some(text) => {
                let c_string = CString::new(text.as_str()).unwrap();
                c_string.into_raw()
            }
            None => ptr::null_mut(),
        }
    }
}

// Новая функция: получает атрибут элемента
#[no_mangle]
pub extern "C" fn html_get_element_attribute(
    parser: *mut HtmlParser, 
    element_index: usize,
    attr_name: *const c_char
) -> *mut c_char {
    if parser.is_null() || attr_name.is_null() {
        return ptr::null_mut();
    }
    
    unsafe {
        let attr_name_str = CStr::from_ptr(attr_name).to_string_lossy();
        match (*parser).get_element_attribute(element_index, &attr_name_str) {
            Some(value) => {
                let c_string = CString::new(value.as_str()).unwrap();
                c_string.into_raw()
            }
            None => ptr::null_mut(),
        }
    }
}

// Новая функция: получает количество атрибутов элемента
#[no_mangle]
pub extern "C" fn html_get_element_attribute_count(
    parser: *mut HtmlParser, 
    element_index: usize
) -> usize {
    if parser.is_null() {
        return 0;
    }
    unsafe {
        (*parser).get_element_attribute_count(element_index)
    }
}

// Новая функция: получает имя атрибута по индексу
#[no_mangle]
pub extern "C" fn html_get_element_attribute_name(
    parser: *mut HtmlParser, 
    element_index: usize,
    attr_index: usize
) -> *mut c_char {
    if parser.is_null() {
        return ptr::null_mut();
    }
    
    unsafe {
        match (*parser).get_element_attribute_name(element_index, attr_index) {
            Some(name) => {
                let c_string = CString::new(name.as_str()).unwrap();
                c_string.into_raw()
            }
            None => ptr::null_mut(),
        }
    }
}

// Старая функция для обратной совместимости
#[no_mangle]
pub extern "C" fn html_parse_string(
    parser: *mut HtmlParser,
    html: *const c_char,
) -> *mut c_char {
    if parser.is_null() || html.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let parser_ref = &mut *parser;
        let html_str = CStr::from_ptr(html).to_string_lossy();
        
        match parser_ref.parse(&html_str) {
            Ok(_) => {
                // Возвращаем "OK" вместо JSON
                let c_string = CString::new("OK").unwrap();
                c_string.into_raw()
            }
            Err(_) => ptr::null_mut(),
        }
    }
}

// Структура для асинхронной загрузки
pub struct AsyncFetchHandle {
    pub handle: std::thread::JoinHandle<Option<String>>,
}

#[no_mangle]
pub extern "C" fn network_fetch_url_async(url: *const c_char) -> *mut AsyncFetchHandle {
    if url.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let url_str = CStr::from_ptr(url).to_string_lossy().to_string();
        
        let handle = std::thread::spawn(move || {
            let rt = match tokio::runtime::Runtime::new() {
                Ok(rt) => rt,
                Err(_) => return None,
            };
            
            rt.block_on(async move {
                let client = reqwest::Client::builder()
                    .timeout(std::time::Duration::from_secs(10))
                    .build()
                    .ok()?;
                    
                let resp = client.get(&url_str).send().await.ok()?;
                let text = resp.text().await.ok()?;
                Some(text)
            })
        });
        
        Box::into_raw(Box::new(AsyncFetchHandle { handle }))
    }
}

#[no_mangle]
pub extern "C" fn network_fetch_url_check(fetch_handle: *mut AsyncFetchHandle) -> i32 {
    if fetch_handle.is_null() {
        return -1; // Ошибка
    }
    
    unsafe {
        let handle_ref = &*fetch_handle;
        if handle_ref.handle.is_finished() {
            1 // Готово
        } else {
            0 // Еще загружается
        }
    }
}

#[no_mangle]
pub extern "C" fn network_fetch_url_result(fetch_handle: *mut AsyncFetchHandle) -> *mut c_char {
    if fetch_handle.is_null() {
        return ptr::null_mut();
    }
    
    unsafe {
        let handle_box = Box::from_raw(fetch_handle);
        match handle_box.handle.join() {
            Ok(Some(text)) => {
                let c_string = CString::new(text).unwrap();
                c_string.into_raw()
            }
            _ => ptr::null_mut(),
        }
    }
}

// Старая синхронная функция для обратной совместимости
#[no_mangle]
pub extern "C" fn network_fetch_url(url: *const c_char) -> *mut c_char {
    if url.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let url_str = CStr::from_ptr(url).to_string_lossy();
        
        // Синхронный вызов с таймаутом
        let rt = tokio::runtime::Runtime::new().unwrap();
        let result = rt.block_on(async move {
            let client = reqwest::Client::builder()
                .timeout(std::time::Duration::from_secs(5))
                .build()?;
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

// Функция для загрузки изображений
#[no_mangle]
pub extern "C" fn network_fetch_image(url: *const c_char) -> *mut c_char {
    if url.is_null() {
        return ptr::null_mut();
    }

    unsafe {
        let url_str = CStr::from_ptr(url).to_string_lossy();
        
        // Синхронная загрузка изображения с таймаутом
        let rt = tokio::runtime::Runtime::new().unwrap();
        let result = rt.block_on(async move {
            let client = reqwest::Client::builder()
                .timeout(std::time::Duration::from_secs(5))
                .build()?;
                
            let resp = client.get(&*url_str).send().await?;
            let bytes = resp.bytes().await?;
            
            // Конвертируем в base64 для передачи в C++
            let base64_data = base64::encode(&bytes);
            Ok::<String, Box<dyn std::error::Error>>(base64_data)
        });

        match result {
            Ok(base64_data) => {
                let c_string = CString::new(base64_data).unwrap();
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
