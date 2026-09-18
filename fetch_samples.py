#!/usr/bin/env python3
import requests
from bs4 import BeautifulSoup
import os
import re
import time

def get_cses_samples():
    base_url = "https://cses.fi"
    list_url = f"{base_url}/problemset/list/"
    
    print(f"Fetching problem list from {list_url}...")
    headers = {'User-Agent': 'Mozilla/5.0'}
    r = requests.get(list_url, headers=headers)
    r.raise_for_status()
    soup = BeautifulSoup(r.text, 'html.parser')
    
    links = []
    # CSES task links look like <a href="/problemset/task/1068">Weird Algorithm</a>
    for a in soup.find_all('a', href=True):
        href = a['href']
        if href.startswith('/problemset/task/'):
            # Some links might be duplicates or different views, but unique task id usually maps 1:1
            links.append((a.text.strip(), href))
            
    # Remove duplicates preserving order
    seen = set()
    unique_links = []
    for name, href in links:
        if href not in seen:
            seen.add(href)
            unique_links.append((name, href))
            
    print(f"Found {len(unique_links)} unique problems.")
    
    # Ensure root tests directory exists
    os.makedirs('tests', exist_ok=True)
    
    for name, href in unique_links:
        # Normalize problem name just like our autograder does
        problem_name = re.sub(r'[ \-_]', '', name).lower()
        test_dir = os.path.join('tests', problem_name)
        os.makedirs(test_dir, exist_ok=True)
        
        task_url = base_url + href
        print(f"Fetching samples for '{name}'...")
        
        try:
            tr = requests.get(task_url, headers=headers)
            tr.raise_for_status()
            tsoup = BeautifulSoup(tr.text, 'html.parser')
            
            inputs = []
            outputs = []
            
            # Look for Input/Output paragraphs and grab the following code block
            for tag in tsoup.find_all(['p', 'h3', 'h4', 'div']):
                text = tag.get_text().strip().lower()
                
                # Check if it's indicating Input
                if text.startswith('input'):
                    code_block = tag.find_next_sibling('pre')
                    if code_block and code_block.code:
                        inputs.append(code_block.code.get_text())
                    elif code_block: # sometimes no code tag inside pre
                        inputs.append(code_block.get_text())
                        
                # Check if it's indicating Output
                elif text.startswith('output'):
                    code_block = tag.find_next_sibling('pre')
                    if code_block and code_block.code:
                        outputs.append(code_block.code.get_text())
                    elif code_block:
                        outputs.append(code_block.get_text())

            # Write the pairs to .in and .out files
            cases_found = min(len(inputs), len(outputs))
            if cases_found == 0:
                print(f"  -> No examples found for '{name}'")
                continue
                
            for i in range(cases_found):
                in_path = os.path.join(test_dir, f"{i+1}.in")
                out_path = os.path.join(test_dir, f"{i+1}.out")
                
                with open(in_path, "w") as f:
                    f.write(inputs[i])
                with open(out_path, "w") as f:
                    f.write(outputs[i])
                    
            print(f"  -> Saved {cases_found} sample(s).")
            
        except Exception as e:
            print(f"  -> Failed to fetch '{name}': {e}")
            
        # polite delay
        time.sleep(0.5)

if __name__ == "__main__":
    get_cses_samples()
