async function exec(resID = "res", stepsID = "steps") {
  const func = document.getElementById("function").value;
  const a = document.getElementById("intervallo_a").value;
  const b = document.getElementById("intervallo_b").value;
  const approx = document.getElementById("precisione").value;
  const method = document.getElementById("method").value;

  if(a == b){
    alert("inserire un intervallo valido!");
    return;
  }

  if(!mathEngine) mathEngine = await loadNumericEngine();

  try{
    const calculus = mathEngine[method];

    if(typeof calculus !== "function"){
      alert("errore: metodo non trovato :/");
      return;
    }
    const risultato = await Reflect.apply(calculus, mathEngine, [func, a, b, approx]);

    const res = document.getElementById(resID);
    res.value = risultato.result;

    const steps = document.getElementById(stepsID);
    steps.value = risultato.steps;

    res.parentElement.hidden = false;
    
  }catch(error){
    alert("errore: " + mathEngine.getExceptionMessage(error)[1]);
    return;
  }
}

let mathEngine = null;