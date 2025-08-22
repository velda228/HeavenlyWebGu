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
        // Упрощенный HTML парсер
        self.elements.clear();
        
        // Простой парсинг тегов
        let mut current_tag = String::new();
        let mut current_attributes = HashMap::new();
        let mut current_text = String::new();
        let mut in_tag = false;
        let mut in_attribute = false;
        let mut attribute_name = String::new();
        let mut attribute_value = String::new();
        
        let chars: Vec<char> = html.chars().collect();
        let mut i = 0;
        
        while i < chars.len() {
            let ch = chars[i];
            
            match ch {
                '<' => {
                    if in_tag {
                        // Закрываем предыдущий тег
                        if !current_tag.is_empty() {
                            self.elements.push(HtmlElement {
                                tag_name: current_tag.clone(),
                                attributes: current_attributes.clone(),
                                text_content: current_text.clone(),
                                children: Vec::new(),
                            });
                        }
                    }
                    
                    in_tag = true;
                    current_tag.clear();
                    current_attributes.clear();
                    current_text.clear();
                    in_attribute = false;
                    attribute_name.clear();
                    attribute_value.clear();
                }
                '>' => {
                    in_tag = false;
                    in_attribute = false;
                    
                    // Добавляем тег
                    if !current_tag.is_empty() {
                        self.elements.push(HtmlElement {
                            tag_name: current_tag.clone(),
                            attributes: current_attributes.clone(),
                            text_content: current_text.clone(),
                            children: Vec::new(),
                        });
                    }
                }
                ' ' => {
                    if in_tag && !current_tag.is_empty() {
                        in_attribute = true;
                    }
                }
                '=' => {
                    if in_attribute {
                        // Начинаем читать значение атрибута
                        i += 1;
                        if i < chars.len() && chars[i] == '"' {
                            i += 1;
                            while i < chars.len() && chars[i] != '"' {
                                attribute_value.push(chars[i]);
                                i += 1;
                            }
                            if i < chars.len() && chars[i] == '"' {
                                current_attributes.insert(attribute_name.clone(), attribute_value.clone());
                                attribute_name.clear();
                                attribute_value.clear();
                                in_attribute = false;
                            }
                        }
                    }
                }
                _ => {
                    if in_tag {
                        if in_attribute {
                            attribute_name.push(ch);
                        } else {
                            current_tag.push(ch);
                        }
                    } else {
                        current_text.push(ch);
                    }
                }
            }
            
            i += 1;
        }
        
        // Сериализуем в JSON
        let json = serde_json::to_string(&self.elements)?;
        Ok(json)
    }

    pub fn get_elements_by_tag(&self, _tag_name: &str) -> Vec<&HtmlElement> {
        // TODO: Реализовать поиск по тегу
        Vec::new()
    }

    pub fn get_element_by_id(&self, _id: &str) -> Option<&HtmlElement> {
        // TODO: Реализовать поиск по ID
        None
    }
}
