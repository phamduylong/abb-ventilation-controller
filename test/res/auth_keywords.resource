*** Settings ***
Documentation     Custom keyword to test authenticate functions 
Library           SeleniumLibrary
Library           FakerLibrary  locale=fi_FI


*** Variables ***
${RAND USERNAME}
${RAND PASSWORD}


*** Keywords ***
Get Random Username 
    [Documentation]       Generate random username
    ${RAND USERNAME}=     FakerLibrary.User Name
    [Return]              ${RAND USERNAME}
Get Random Password
    [Documentation]       Generate random password
    ${RAND PASSWORD}=     FakerLibrary.Password    12    False    True    True    True
    [Return]              ${RAND PASSWORD}

Open Browser To Page And Check Title    
    [Documentation]     Open a page and check page title
    [Arguments]         ${url}    ${title}
    Open Browser        ${url}    ${BROWSER}
    Title Should Be     ${title}      

Input Username
    [Documentation]     Input username to the username field inside of the form
    [Arguments]         ${username}
    Input Text          username    ${username}

Input Password
    [Documentation]     Input password to the password field inside of the form
    [Arguments]         ${password}
    Input Text          password    ${password}

Submit Credentials
    [Documentation]     Submit the form
    Submit Form        

Auto Mode Page Should Be Open
    [Documentation]     Current location of the site should be at the auto mode page (Default)
    Title Should Be     Auto Mode

Home Page Should Be Open
    [Documentation]     Current route should be at login page
    Title Should Be     Home Page

Pop Up Text Signup Successful
    [Documentation]             A pop up text should be displayed to notify user they have signed up successfully
    Element Text Should Be      err     New User Created!

Pop Up Text Signup Failed
    [Documentation]     A pop up text should be displayed to notify user signing up process has failed
    Element Text Should Not Be      err     New User Created!

Pop Up Text Incorrect Credentials
    [Documentation]     A pop up text should be displayed to notify user they have entered incorrect credentials
    Element Text Should Be      err     Username or password is incorrect