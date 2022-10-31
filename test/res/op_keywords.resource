*** Settings ***
Documentation     Custom keyword to test operation of the page
Library           SeleniumLibrary
Resource		                            .${/}res${/}auth_keywords.resource

*** Variables ***
${LOG OUT BUTTON ID}                        logout-btn

*** Keywords ***
Login
    Input Username                          newuser
    Input Account Password                  Ziggs2305
    Submit Credentials
    Auto Mode Page Should Be Open

Logout
    Click Element                           ${LOG OUT BUTTON ID}

