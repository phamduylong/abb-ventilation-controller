*** Settings ***
Documentation                               Testing authenticate functions
Library                                     SeleniumLibrary
Library                                     FakerLibrary  locale=fi_FI
Resource		                            .${/}res${/}auth_keywords.resource
Test Setup                                  Open Browser To Page And Check Title    ${LOGIN URL}   ${LOGIN PAGE TITLE}
Test Teardown                               Close Browser

*** Variables ***
${RAND USERNAME}
${RAND PASSWORD}


*** Test Cases ***

Invalid Login Wrong Username And Password
    [Documentation]                         Login case with both username and password incorrect
    Input Username                          dasfdsaf
    Input Account Password                  dahfasdfZ123
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Incorrect Credentials
    Sleep                                   2 seconds


Invalid Login Wrong Password
    [Documentation]                         Login case with an existing user but wrong password
    Input Username                          newuser
    Input Account Password                  dahfasdfZ123
    Submit Credentials
    Home Page Should Be Open
    Pop Up Text Incorrect Credentials
    Sleep                                   2 seconds


Valid Login
    [Documentation]                         Valid login with matching credentials case
    Input Username                          newuser
    Input Account Password                  Ziggs2305
    Submit Credentials
    Auto Mode Page Should Be Open
    Sleep                                   2 seconds

