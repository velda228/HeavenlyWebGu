use reqwest::Client;
use serde::{Serialize, Deserialize};
use std::collections::HashMap;
use tokio::runtime::Runtime;
use std::error::Error;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HttpResponse {
    pub status: u16,
    pub headers: HashMap<String, String>,
    pub body: String,
    pub url: String,
}

#[derive(Debug)]
pub struct NetworkManager {
    client: Client,
    runtime: Runtime,
}

impl NetworkManager {
    pub fn new() -> Result<Self, Box<dyn Error>> {
        let client = Client::builder()
            .user_agent("HeavenlyWebGu/1.0")
            .build()?;
        
        let runtime = Runtime::new()?;
        
        Ok(Self { client, runtime })
    }

    pub fn fetch_url(&self, url: &str) -> Result<HttpResponse, Box<dyn Error>> {
        let client = self.client.clone();
        let url = url.to_string();
        
        let response = self.runtime.block_on(async move {
            let resp = client.get(&url).send().await?;
            let status = resp.status().as_u16();
            let headers = resp.headers().clone();
            let body = resp.text().await?;
            
            Ok::<HttpResponse, Box<dyn Error>>(HttpResponse {
                status,
                headers: headers
                    .iter()
                    .map(|(k, v)| (k.to_string(), v.to_str().unwrap_or("").to_string()))
                    .collect(),
                body,
                url,
            })
        })?;
        
        Ok(response)
    }

    pub fn fetch_url_with_headers(
        &self,
        url: &str,
        headers: HashMap<String, String>,
    ) -> Result<HttpResponse, Box<dyn Error>> {
        let client = self.client.clone();
        let url = url.to_string();
        
        let response = self.runtime.block_on(async move {
            let mut request = client.get(&url);
            
            for (key, value) in headers {
                request = request.header(key, value);
            }
            
            let resp = request.send().await?;
            let status = resp.status().as_u16();
            let resp_headers = resp.headers().clone();
            let body = resp.text().await?;
            
            Ok::<HttpResponse, Box<dyn Error>>(HttpResponse {
                status,
                headers: resp_headers
                    .iter()
                    .map(|(k, v)| (k.to_string(), v.to_str().unwrap_or("").to_string()))
                    .collect(),
                body,
                url,
            })
        })?;
        
        Ok(response)
    }

    pub fn post_data(
        &self,
        url: &str,
        data: &str,
        content_type: &str,
    ) -> Result<HttpResponse, Box<dyn Error>> {
        let client = self.client.clone();
        let url = url.to_string();
        let data = data.to_string();
        let content_type = content_type.to_string();
        
        let response = self.runtime.block_on(async move {
            let resp = client
                .post(&url)
                .header("Content-Type", content_type)
                .body(data)
                .send()
                .await?;
            
            let status = resp.status().as_u16();
            let headers = resp.headers().clone();
            let body = resp.text().await?;
            
            Ok::<HttpResponse, Box<dyn Error>>(HttpResponse {
                status,
                headers: headers
                    .iter()
                    .map(|(k, v)| (k.to_string(), v.to_str().unwrap_or("").to_string()))
                    .collect(),
                body,
                url,
            })
        })?;
        
        Ok(response)
    }
}
