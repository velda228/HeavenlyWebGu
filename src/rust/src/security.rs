use ring::digest::{digest, SHA256};
use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SecurityInfo {
    pub is_https: bool,
    pub certificate_valid: bool,
    pub content_security_policy: Option<String>,
    pub x_frame_options: Option<String>,
    pub x_content_type_options: Option<String>,
    pub strict_transport_security: Option<String>,
}

#[derive(Debug)]
pub struct SecurityManager {
    trusted_certificates: Vec<Vec<u8>>,
    blocked_domains: Vec<String>,
}

impl SecurityManager {
    pub fn new() -> Self {
        Self {
            trusted_certificates: Vec::new(),
            blocked_domains: vec![
                "malware.example.com".to_string(),
                "phishing.example.com".to_string(),
            ],
        }
    }

    pub fn check_url_security(&self, url: &str) -> SecurityInfo {
        let is_https = url.starts_with("https://");
        
        SecurityInfo {
            is_https,
            certificate_valid: is_https, // Упрощенная проверка
            content_security_policy: None,
            x_frame_options: None,
            x_content_type_options: None,
            strict_transport_security: None,
        }
    }

    pub fn is_domain_blocked(&self, domain: &str) -> bool {
        self.blocked_domains.iter().any(|blocked| domain.contains(blocked))
    }

    pub fn calculate_hash(&self, data: &[u8]) -> String {
        let hash = digest(&SHA256, data);
        hex::encode(hash.as_ref())
    }

    pub fn validate_content_security_policy(
        &self,
        policy: &str,
        resource_url: &str,
    ) -> bool {
        // Простая проверка CSP
        if policy.contains("default-src 'self'") {
            return resource_url.starts_with("https://") || resource_url.starts_with("data:");
        }
        true
    }

    pub fn sanitize_html(&self, html: &str) -> String {
        // Базовая санитизация HTML
        let mut sanitized = html.to_string();
        
        // Убираем потенциально опасные теги
        let dangerous_tags = [
            "<script", "</script>", "<iframe", "</iframe>", 
            "<object", "</object>", "<embed", "</embed>"
        ];
        
        for tag in &dangerous_tags {
            sanitized = sanitized.replace(tag, "");
        }
        
        // Убираем javascript: ссылки
        sanitized = sanitized.replace("javascript:", "");
        
        sanitized
    }

    pub fn check_xss_vulnerability(&self, input: &str) -> bool {
        let xss_patterns = [
            "<script", "javascript:", "onload=", "onerror=",
            "onclick=", "onmouseover=", "eval(", "alert(",
        ];
        
        xss_patterns.iter().any(|pattern| input.contains(pattern))
    }
}
