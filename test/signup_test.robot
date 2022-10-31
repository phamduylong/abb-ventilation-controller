*** Settings ***
Documentation                               Testing authenticate functions
Library                                     SeleniumLibrary
Library                                     FakerLibrary  locale=fi_FI
Resource		                            .${/}res${/}auth_keywords.resource
Test Setup                                  Open Browser To Page And Check Title    ${SIGNUP URL}   ${SIGNUP PAGE TITLE}
Test Teardown                               Close Browser

*** Variables ***
${RAND USERNAME}
${RAND PASSWORD}

*** Test Cases ***

Valid Signup
    [Documentation]                         Valid signup with a randomized username and password that match the required regular expression (might fail on rare cases where randomized username had been used previously)
    ${RAND USERNAME}=                       FakerLibrary.User Name
    ${RAND PASSWORD}=                       FakerLibrary.Password    12    False    True    True    True
    Input Username                          ${RAND USERNAME}
    Input Account Password                  ${RAND PASSWORD}
    Sleep                                   2 seconds
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Signup Successful
    Sleep                                   2 seconds

Invalid Signup Duplicate Username
    [Documentation]                         Signing up using an existing username  
    Input Username                          newuser
    ${RAND PASSWORD}=                       Get Random Password
    ${RAND PASSWORD}=                       FakerLibrary.Password    12    False    True    True    True
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Signup Failed
    Sleep                                   2 seconds
