*** Settings ***
Documentation     Testing authenticate functions
Library           SeleniumLibrary
Library           FakerLibrary  locale=fi_FI
Resource		  .${/}res${/}auth_keywords.resource

*** Variables ***
${LOGIN URL}      http://localhost:3000
${SIGNUP URL}     http://localhost:3000/signup
${BROWSER}        Edge
${RAND USERNAME}
${RAND PASSWORD}

*** Test Cases ***
Valid Login
    Open Browser To Page And Check Title    ${LOGIN URL}        Home Page
    Input Username                          longphd
    Input Password                          Ziggs2305
    Submit Credentials
    Auto Mode Page Should Be Open
    Sleep                                   2 seconds
    [Teardown]    Close Browser

Valid Signup
    Open Browser To Page And Check Title    ${SIGNUP URL}       Sign Up
    Get Random Username
    Get Random Password
    Input Username                          ${RAND USERNAME}
    Input Password                          ${RAND PASSWORD}
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Signup Successful
    Sleep                                   2 seconds
    [Teardown]    Close Browser

Invalid Login
    Open Browser To Page And Check Title    ${LOGIN URL}        Home Page
    Input Username                          dasfdsaf
    Input Password                          dahfasdfZ123
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Incorrect Credentials
    Sleep                                   2 seconds
    [Teardown]    Close Browser

Invalid Signup Duplicate Username
    Open Browser To Page And Check Title    ${SIGNUP URL}       Sign Up
    Input Username                          newuser
    Input Password                          ${RAND PASSWORD}
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Signup Failed
    Sleep                                   2 seconds
    [Teardown]    Close Browser

