/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



/**
 * @state @Component/ViewPU variable decorator
 * 
 * local init required - transpiler needs to support
 * no init or update form parent - transpiler needs to support
 * new value assignment allowed = has setter 
 *
 * part of SDK
 * @from 12
 *
 */
/*
const state = (target: Object, propertyKey: string) => {
    ObserveV3.addVariableDecoMeta(target, propertyKey, "@state");
    return trackInternal(target, propertyKey);
  }
  */
  
  /**
   * @param class property decorator 
   * 
   * local init optional - transpiler needs to support
   * init and update form parent is mandatory when no local init, otherwise optional - transpiler needs to support
   * new value assignment not allowed = has no setter. For update from parent @Component, 
   *               transpiler calls ViewPU.updateParam(paramName).
   * 
   * @param target  ViewPU class prototype object
   * @param propertyKey  class property name
   * 
   * turns given property into getter and setter functions
   * adds property target[storeProp] as the backing store
   *
   * part of SDK
   * @from 12
   *
   */
  /*
  const param = (target : Object, propertyKey : string) => {
    ObserveV3.addVariableDecoMeta(target, propertyKey, "@param");
  
    let storeProp = ObserveV3.OB_PREFIX + propertyKey
    target[storeProp] = target[propertyKey]
    Reflect.defineProperty(target, propertyKey, {
      get() {
        ObserveV3.getObserve().addRef(this, propertyKey)
        return ObserveV3.autoProxyObject(this, ObserveV3.OB_PREFIX + propertyKey)
      },
      set(_) {
        stateMgmtConsole.applicationError(`@param ${propertyKey.toString()}: can not assign a new value, application error.`)
        return;
      },
      // @param can not be assigned, no setter
      enumerable: true
    })
  } // param
  */
  
  /**
   * @event @Component/ViewPU variable decorator
   * 
   * @param target 
   * @param propertyKey 
   */
  
  /*
  const event = (target, propertyKey) => {
    ObserveV3.addVariableDecoMeta(target, propertyKey, "@event");
    target[propertyKey] = () => {};
  }
  */

class ProvideConsumeUtilV3 {
  private static readonly ALIAS_PREFIX='___pc_alias_';

  /**
   * Helper function to add meta data about @provide and @comnsume decorators to ViewPU
   * similar to @see addVariableDecoMeta, but adds the alias to allow search from @consume for @provide counterpart
   * @param proto prototype object of application class derived from ViewPU 
   * @param varName decorated variable
   * @param deco "@state", "@event", etc (note "@model" gets transpiled in "@param" and "@event")
   */
  public static addProvideConsumeVariableDecoMeta(proto: Object, varName: string, aliasName: string, deco: "@provide" | "@consume"): void {
    // add decorator meta data to prototype
    const meta = proto[ObserveV3.V3_DECO_META] ??= {};
    // note: aliasName is the actual alias not the prefixed version
    meta[varName] = { "deco": deco, "aliasName": aliasName };

    // prefix to avoid name collisions with variable of same name as the alias!
    const aliasProp = ProvideConsumeUtilV3.ALIAS_PREFIX + aliasName;
    meta[aliasProp] = { "varName": varName, "deco": deco }

    // FIXME 
    // when splitting ViewPU and ViewV3
    // use instanceOf. Until then, this is a workaround.
    // any @state, @track, etc V3 event handles this function to return false
    Reflect.defineProperty(proto, "isViewV3", {
      get() { return true; },
      enumerable: false
    }
    );
  }

  public static setupConsumeVarsV3(view : ViewPU) : boolean {
    const meta = view && view[ObserveV3.V3_DECO_META];
    if (!meta) {
      return;
    }
  
    for (const [key, value] of Object.entries(meta)) {
      if (value["deco"]=="@consume" && value["varName"]) {
        const prefixedAliasName = key;
        let result = ProvideConsumeUtilV3.findProvide(view, prefixedAliasName);
        if (result && result[0] && result[1]) {
          ProvideConsumeUtilV3.connectConsume2Provide(view, value["varName"], result[0], result[1])
        } else {
          ProvideConsumeUtilV3.defineConsumeWithoutProvide(view, value["varName"]);
        }
      }
    }
  }

  /**
  * v3: find a @provide'ed variable from its nearest ancestor ViewPU.
  * @param searchingAliasName The key name to search for.
  * @returns A tuple containing the ViewPU instance where the provider is found
  * and the provider name
  * If root @Component reached without finding, returns undefined.
  */
  private static findProvide(view: ViewPU, searchingPrefixedAliasName: string): [ViewPU, string] | undefined {
    let checkView = view?.getParent();
    while (checkView) {
      const meta = checkView.constructor?.prototype[ObserveV3.V3_DECO_META]
      if (meta && meta[searchingPrefixedAliasName]) {
        const aliasMeta = meta[searchingPrefixedAliasName];
        const providedVarName: string | undefined = aliasMeta && (aliasMeta["deco"] == "@provide" ? aliasMeta["varName"] : undefined);

        if (providedVarName) {
          stateMgmtConsole.debug(`findProvide: success: ${checkView.debugInfo__()} has matching @provide('${searchingPrefixedAliasName.substring(ProvideConsumeUtilV3.ALIAS_PREFIX.length)}') ${providedVarName}`);
          return [checkView, providedVarName];
        }
      }
      checkView = checkView.getParent();
    }; // while
    stateMgmtConsole.debug(`findProvide:  ${view.debugInfo__()} @consume('${searchingPrefixedAliasName.substring(ProvideConsumeUtilV3.ALIAS_PREFIX.length)}'), no matching @provide found amongst ancestor @Components`);
    return undefined;
  }

  private static connectConsume2Provide(consumeView : ViewPU, consumeVarName : string, provideView : ViewPU, provideVarName: string) {
    stateMgmtConsole.debug(`connectConsume2PRovide: Connect ${consumeView.debugInfo__()} '@consume ${consumeVarName}' to ${provideView.debugInfo__()} '@provide ${provideVarName}'`);

    // weakref provideView ?
    //const storeProvide = ObserveV3.OB_PREFIX + provideVarName;
    const weakView = new WeakRef<ViewPU>(provideView);
    const provideViewName = provideView.constructor?.name;
    Reflect.defineProperty(consumeView, consumeVarName, {
      get() {
        stateMgmtConsole.propertyAccess(`@consume ${consumeVarName} get`)
        ObserveV3.getObserve().addRef(this, consumeVarName);
        const view=weakView.deref();
        if (!view) {
          const error=`${this.debugInfo__()}: get() on @consume ${consumeVarName}: providing @ComponentV2 ${provideViewName} no longer exists. Application error.` 
          stateMgmtConsole.error(error);
          throw new Error(error);
        }
        return view[provideVarName];
      },
      set(val) {
        // If the object has not been observed, you can directly assign a value to it. This improves performance.
        stateMgmtConsole.propertyAccess(`@consume ${consumeVarName} set`)
        const view=weakView.deref();
        if (!view) {
          const error=`${this.debugInfo__()}: set() on @consume ${consumeVarName}: providing @ComponentV2 ${provideViewName} no longer exists. Application error.` 
          stateMgmtConsole.error(error);
          throw new Error(error);
        }

        if (val !== view[provideVarName]) {
          stateMgmtConsole.propertyAccess(`@consume ${consumeVarName} valueChanged`);
          view[provideVarName] = val;
        if (this[ObserveV3.SYMBOL_REFS]) { // This condition can improve performance.
            ObserveV3.getObserve().fireChange(this, consumeVarName);
          }
        }
      },
      enumerable: true
    })
  }
  
  private static defineConsumeWithoutProvide(consumeView : ViewPU, consumeVarName : string) {
    stateMgmtConsole.debug(`defineConsumeWithoutProvide: ${consumeView.debugInfo__()} @consume ${consumeVarName} does not have @provide counter part, uses local init value`);

    const storeProp = ObserveV3.OB_PREFIX + consumeVarName;
    consumeView[storeProp] = consumeView[consumeVarName]; // use local init value, also as backing store
    Reflect.defineProperty(consumeView, consumeVarName, {
      get() {
        ObserveV3.getObserve().addRef(this, consumeVarName)
        return ObserveV3.autoProxyObject(this, ObserveV3.OB_PREFIX + consumeVarName)
      },
      set(val) {
        if (val !== this[storeProp]) {
          this[storeProp] = val
        if (this[ObserveV3.SYMBOL_REFS]) { // This condition can improve performance.
            ObserveV3.getObserve().fireChange(this, consumeVarName)
          }
        }
      },
      enumerable: true
    })
  }
}


/**
 * ViewPU variable decorator @provide(alias> : string) varName
 * @param alias defaults to varName
 * @returns 
 */
const provide = (aliasName?: string) => {
  return (target: Object, varName: string) => {
    const providedUnderName: string = aliasName || varName;
    ProvideConsumeUtilV3.addProvideConsumeVariableDecoMeta(target, varName, providedUnderName, "@provide");
    trackInternal(target, varName);
  }
} // @provide

const consume = (aliasName?: string ) => {
  return (target: ViewPU, varName: string) => {
    const searchForProvideWithName: string = aliasName || varName;

    // redefining the property happens when ViewPU gets constructed
    // and @Consume gets connected to @provide counterpart
    ProvideConsumeUtilV3.addProvideConsumeVariableDecoMeta(target, varName, searchForProvideWithName, "@consume");
}
}



// The prop parameter is not carried when the component is updated.
// FIXME what is the purpose of this ?
/*
let updateChild = ViewPU.prototype["updateStateVarsOfChildByElmtId"];
ViewPU.prototype["updateStateVarsOfChildByElmtId"] = function (elmtId, params) {
  updateChild?.call(this, elmtId, params);
  let child = this.getChildById(elmtId);
  if (child) {
    let realParams = child.paramsGenerator_ ? child.paramsGenerator_() : params
    for (let k in realParams) {
      if (ObserveV3.OB_PREFIX + k in child) {
        child[k] = realParams[k];
      }
    }
  }
}
*/
