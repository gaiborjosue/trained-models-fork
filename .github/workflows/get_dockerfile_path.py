import os
import re
import json

def extract_organization_name(pull_request_description):

    pr_data = json.loads(pull_request_description)

    pr_body = pr_data.get('body')

    # Regular expression pattern to match the 'Organization Name' line in the template
    pattern = r'Organization Name:\s+(.*)'

    # Search for the pattern in the pull request description
    match = re.search(pattern, pr_body, re.IGNORECASE)

    if match:
        organization_name = match.group(1).strip()
        # Return the organization name just as it is, with upper and lower case letters if any
        return organization_name
    return None
def get_latest_model_name():
    pull_request_description = os.environ.get('INPUT PULL_REQUEST_BODY')


    org_folder = extract_organization_name(pull_request_description)
    
    # Cd into the org folder
    os.chdir(org_folder)

    # Assign current directory to model_n
    model_n = os.getcwd()

    if model_n == 'objects':
        exit()
    else:
        return model_n
def get_dockerfile_path(model_folder):
    for root, _, files in os.walk(model_folder):
        for file in files:
            if file.lower() == "dockerfile":
                return os.path.join(root)
    return None


if __name__ == "__main__":
    model_folder = get_latest_model_name()

    dockerfile_path = get_dockerfile_path(model_folder)
    if dockerfile_path:
        print(dockerfile_path)
    else:
        print("Dockerfile not found.")