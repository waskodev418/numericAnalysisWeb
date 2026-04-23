async function bisezione(resID = "res", stepsID = "steps") {
  const func = document.getElementById("function").value;
  const a = document.getElementById("intervallo_a").value;
  const b = document.getElementById("intervallo_b").value;
  const approx = document.getElementById("precisione").value;

  const mathEngine = await loadNumericEngine();
  try{
    const risultato = mathEngine.bisezione(func, a, b, approx);

    const res = document.getElementById(resID);
    res.value = risultato.result;
    res.hidden = false;

    const steps = document.getElementById(stepsID);
    steps.value = risultato.steps;
    steps.hidden = false;
    
  }catch(error){
    alert("errore: " + mathEngine.getExceptionMessage(error)[1]);
    return;
  }
}