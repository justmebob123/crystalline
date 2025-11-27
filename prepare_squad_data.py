#!/usr/bin/env python3
"""
Extract text from SQuAD dataset for CLLM training
"""
import json
import sys

def extract_squad_text(input_file, output_file):
    """Extract all text from SQuAD dataset"""
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    texts = []
    total_chars = 0
    
    for article in data['data']:
        title = article['title']
        texts.append(f"# {title}\n\n")
        
        for paragraph in article['paragraphs']:
            context = paragraph['context']
            texts.append(context + "\n\n")
            total_chars += len(context)
            
            # Add questions and answers
            for qa in paragraph['qas']:
                question = qa['question']
                texts.append(f"Q: {question}\n")
                
                if not qa.get('is_impossible', False) and qa.get('answers'):
                    answer = qa['answers'][0]['text']
                    texts.append(f"A: {answer}\n\n")
    
    # Write to output
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(''.join(texts))
    
    print(f"Extracted {len(texts)} text segments")
    print(f"Total characters: {total_chars:,}")
    print(f"Output written to: {output_file}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python prepare_squad_data.py <input.json> <output.txt>")
        sys.exit(1)
    
    extract_squad_text(sys.argv[1], sys.argv[2])