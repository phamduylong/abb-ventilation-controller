*** Settings ***
Documentation     Testing authenticate functions
Library           SeleniumLibrary

*** Variables ***
${LOGIN URL}      http://localhost:3000
${SIGNUP URL}     http://localhost:3000/signup
${BROWSER}        Edge
${BROWSER_OPT}    options=add_experimental_option('excludeSwitches', ['enable-logging'])

*** Test Cases ***
Valid Login
    Open Browser To Login Page
    Input Username    longphd
    Input Password    Ziggs2305
    Submit Credentials
    Auto Mode Page Should Be Open
    [Teardown]    Close Browser

*** Keywords ***
Open Browser To Login Page
    Open Browser    ${LOGIN URL}    ${BROWSER}   
    Title Should Be    Home Page

Input Username
    [Arguments]    ${username}
    Input Text    username    ${username}

Input Password
    [Arguments]    ${password}
    Input Text    password    ${password}

Submit Credentials
    Submit Form        

Auto Mode Page Should Be Open
    Title Should Be    Auto Mode