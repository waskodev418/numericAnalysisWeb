function createScientificKeyboard(targetInputId = "function", cont = "keyboard") {
    const container = document.getElementById(cont);
    container.className = 'sci-kbd-container';
    const inputField = document.getElementById(targetInputId);
    
    const keys = [
        ['asin', 'asin(', 'btn-fn'], ['acos', 'acos(', 'btn-fn'], ['atan', 'atan(', 'btn-fn'], ['atan2', 'atan2(', 'btn-fn'],
        ['sin', 'sin(', 'btn-fn'], ['cos', 'cos(', 'btn-fn'], ['tan', 'tan(', 'btn-fn'], ['log', 'log(', 'btn-fn'],
        ['√', 'sqrt(', 'btn-fn'], ['^', '^', 'btn-fn'], ['(', '(', 'btn-fn'], [')', ')', 'btn-fn'],
        ['7', '7', ''], ['8', '8', ''], ['9', '9', ''], ['/', '/', 'btn-op'],
        ['4', '4', ''], ['5', '5', ''], ['6', '6', ''], ['*', '*', 'btn-op'],
        ['1', '1', ''], ['2', '2', ''], ['3', '3', ''], ['-', '-', 'btn-op'],
        ['0', '0', ''], ['.', '.', ''], ['x', 'x', 'btn-x'], ['+', '+', 'btn-op'],
        ['AC', 'CLEAR', 'btn-ac'], ['⌫', 'BACKSPACE', 'btn-del'], [',', ',', 'btn-op']
    ];

    keys.forEach(([label, value, className]) => {
        const btn = document.createElement('button');
        btn.innerText = label;
        btn.type = "button";
        if (className) btn.className = className;

        btn.onclick = () => {
            if (value === 'CLEAR') {
                inputField.value = '';
            } else if (value === 'BACKSPACE') {
                inputField.value = inputField.value.slice(0, -1);
            } else {
                const start = inputField.selectionStart;
                const end = inputField.selectionEnd;
                inputField.setRangeText(value, start, end, 'end');
            }
            inputField.focus();
        };
        container.appendChild(btn);
    });

    return container;
}