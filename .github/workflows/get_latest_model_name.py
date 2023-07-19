import os
import re
import sys

def extract_organization_name(pull_request_description):
    # Regular expression pattern to match the 'Organization Name' line in the template
    pattern = r'Organization Name:\s+(.*)'

    # Search for the pattern in the pull request description
    match = re.search(pattern, pull_request_description, re.IGNORECASE)

    if match:
        organization_name = match.group(1).strip()
        # Return the organization name just as it is, with upper and lower case letters if any
        return organization_name
    return None

def get_latest_model_name():
    pull_request_description = os.environ.get('pr_body')

    org_folder = extract_organization_name(pull_request_description)
    
    # Cd into the org folder
    os.chdir(org_folder)

    # List all files
    model_n = os.listdir('.')[0]

    if model_n == 'objects':
        exit()
    else:
        return model_n

if __name__ == "__main__":

    print(get_latest_model_name())