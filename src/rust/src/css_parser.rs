
use std::collections::HashMap;
use serde::{Serialize, Deserialize};
use std::error::Error;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CssRule {
    pub selector: String,
    pub properties: HashMap<String, String>,
}

#[derive(Debug)]
pub struct CssParser {
    rules: Vec<CssRule>,
}

impl CssParser {
    pub fn new() -> Self {
        Self { rules: Vec::new() }
    }

    pub fn parse(&mut self, css: &str) -> Result<String, Box<dyn Error>> {
        // Упрощенный CSS парсер
        let lines: Vec<&str> = css.lines().collect();
        let mut current_selector = String::new();
        let mut current_properties = HashMap::new();
        
        for line in lines {
            let line = line.trim();
            
            if line.is_empty() {
                continue;
            }
            
            // Если строка заканчивается на {, это селектор
            if line.ends_with('{') {
                if !current_selector.is_empty() && !current_properties.is_empty() {
                    // Сохраняем предыдущее правило
                    self.rules.push(CssRule {
                        selector: current_selector.clone(),
                        properties: current_properties.clone(),
                    });
                    current_properties.clear();
                }
                current_selector = line[..line.len()-1].trim().to_string();
            }
            // Если строка содержит :, это свойство
            else if line.contains(':') && !current_selector.is_empty() {
                let parts: Vec<&str> = line.split(':').collect();
                if parts.len() >= 2 {
                    let property = parts[0].trim();
                    let value = parts[1].trim().trim_end_matches(';');
                    current_properties.insert(property.to_string(), value.to_string());
                }
            }
            // Если строка заканчивается на }, закрываем правило
            else if line.ends_with('}') && !current_selector.is_empty() {
                if !current_properties.is_empty() {
                    self.rules.push(CssRule {
                        selector: current_selector.clone(),
                        properties: current_properties.clone(),
                    });
                    current_properties.clear();
                }
                current_selector.clear();
            }
        }
        
        // Сериализуем в JSON
        let json = serde_json::to_string(&self.rules)?;
        Ok(json)
    }

    pub fn get_rules_for_selector(&self, selector: &str) -> Vec<&CssRule> {
        self.rules
            .iter()
            .filter(|rule| rule.selector.contains(selector))
            .collect()
    }

    pub fn get_property_value(&self, selector: &str, property: &str) -> Option<&String> {
        for rule in &self.rules {
            if rule.selector.contains(selector) {
                return rule.properties.get(property);
            }
        }
        None
    }
}
