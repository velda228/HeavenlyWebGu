use std::collections::HashMap;
use serde::{Serialize, Deserialize};
use std::error::Error;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HtmlElement {
    pub tag_name: String,
    pub attributes: HashMap<String, String>,
    pub text_content: String,
    pub children: Vec<HtmlElement>,
}

#[derive(Debug)]
pub struct HtmlParser {
    elements: Vec<HtmlElement>,
}

impl HtmlParser {
    pub fn new() -> Self {
        Self { elements: Vec::new() }
    }

    pub fn parse(&mut self, html: &str) -> Result<String, Box<dyn Error>> {
        // Быстрый HTML парсер с ограничением элементов
        self.elements.clear();
        
        // Ограничиваем количество элементов для производительности
        const MAX_ELEMENTS: usize = 500;
        
        let mut pos = 0;
        let chars: Vec<char> = html.chars().collect();
        
        while pos < chars.len() && self.elements.len() < MAX_ELEMENTS {
            // Ищем открывающий тег
            if let Some(tag_start) = self.find_next_tag_start(&chars, pos) {
                // Извлекаем текст перед тегом (только если он значимый)
                if tag_start > pos {
                    let text_content = chars[pos..tag_start].iter().collect::<String>().trim().to_string();
                    if !text_content.is_empty() && 
                       !text_content.chars().all(|c| c.is_whitespace()) &&
                       text_content.len() > 1 {
                        self.elements.push(HtmlElement {
                            tag_name: "#text".to_string(),
                            attributes: HashMap::new(),
                            text_content,
                            children: Vec::new(),
                        });
                    }
                }
                
                // Парсим тег
                if let Some(tag_end) = self.find_tag_end(&chars, tag_start) {
                    let tag_content = chars[tag_start+1..tag_end].iter().collect::<String>();
                    
                    // Пропускаем технические теги
                    if !tag_content.starts_with('!') && 
                       !tag_content.starts_with('/') &&
                       !tag_content.starts_with("script") &&
                       !tag_content.starts_with("style") &&
                       !tag_content.starts_with("meta") &&
                       !tag_content.starts_with("link") &&
                       !tag_content.starts_with("noscript") {
                        
                        let element = self.parse_tag(&tag_content);
                        if let Some(elem) = element {
                            // Ищем текст после тега (упрощенно)
                            let text_start = tag_end + 1;
                            if let Some(next_tag_start) = self.find_next_tag_start(&chars, text_start) {
                                let text_between = chars[text_start..next_tag_start].iter().collect::<String>().trim().to_string();
                                if !text_between.is_empty() && 
                                   !text_between.chars().all(|c| c.is_whitespace()) &&
                                   text_between.len() > 1 {
                                    let mut element_with_text = elem;
                                    element_with_text.text_content = text_between;
                                    self.elements.push(element_with_text);
                                } else {
                                    self.elements.push(elem);
                                }
                            } else {
                                self.elements.push(elem);
                            }
                        }
                    }
                    pos = tag_end + 1;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        // Сериализуем в JSON
        let json = serde_json::to_string(&self.elements)?;
        Ok(json)
    }
    
    fn find_next_tag_start(&self, chars: &[char], start: usize) -> Option<usize> {
        for i in start..chars.len() {
            if chars[i] == '<' {
                return Some(i);
            }
        }
        None
    }
    
    fn find_tag_end(&self, chars: &[char], start: usize) -> Option<usize> {
        for i in start..chars.len() {
            if chars[i] == '>' {
                return Some(i);
            }
        }
        None
    }
    
    fn parse_tag(&self, tag_content: &str) -> Option<HtmlElement> {
        let parts: Vec<&str> = tag_content.trim().split_whitespace().collect();
        if parts.is_empty() {
            return None;
        }
        
        let tag_name = parts[0].to_lowercase();
        let mut attributes = HashMap::new();
        
        // Простой парсинг атрибутов
        for part in &parts[1..] {
            if let Some(eq_pos) = part.find('=') {
                let key = part[..eq_pos].to_string();
                let value = part[eq_pos+1..].trim_matches('"').trim_matches('\'').to_string();
                attributes.insert(key, value);
            }
        }
        
        Some(HtmlElement {
            tag_name,
            attributes,
            text_content: String::new(),
            children: Vec::new(),
        })
    }

    pub fn get_elements_by_tag(&self, _tag_name: &str) -> Vec<&HtmlElement> {
        // TODO: Реализовать поиск по тегу
        Vec::new()
    }

    pub fn get_element_by_id(&self, _id: &str) -> Option<&HtmlElement> {
        // TODO: Реализовать поиск по ID
        None
    }
    
    // Новые методы для FFI
    
    pub fn get_element_count(&self) -> usize {
        self.elements.len()
    }
    
    pub fn get_element_tag_name(&self, index: usize) -> Option<&String> {
        self.elements.get(index).map(|el| &el.tag_name)
    }
    
    pub fn get_element_text(&self, index: usize) -> Option<&String> {
        self.elements.get(index).map(|el| &el.text_content)
    }
    
    pub fn get_element_attribute(&self, element_index: usize, attr_name: &str) -> Option<&String> {
        self.elements.get(element_index)?.attributes.get(attr_name)
    }
    
    pub fn get_element_attribute_count(&self, element_index: usize) -> usize {
        self.elements.get(element_index).map(|el| el.attributes.len()).unwrap_or(0)
    }
    
    pub fn get_element_attribute_name(&self, element_index: usize, attr_index: usize) -> Option<&String> {
        let element = self.elements.get(element_index)?;
        element.attributes.keys().nth(attr_index)
    }
}
